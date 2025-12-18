/*
* This file is part of Kufibot.
 *
 * Kufibot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kufibot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kufibot. If not, see <https://www.gnu.org/licenses/>.
 */

#include "llama_operator.h"
#include <iostream>
#include <cstring>
#include "../logger.h"

LlamaOperator::LlamaOperator()
{
    _ngl = 99;
    _nCtx = 2048;
    _minP = 0.05f;
    _temp = 0.8f;
    _topK = 50;
    _topP = 0.9;
    _nThreads = 4;
    _model = nullptr;
    _ctx = nullptr;
    _smpl = nullptr;
    _vocab = nullptr;
    _modelLoaded = false;
    _systemMessage = "";
}

LlamaOperator::~LlamaOperator() {
    unloadModel();
}

void LlamaOperator::setOptions(int ngl, int nThreads, int n_ctx, float minP, float temp, int topK,
                               float topP) {
    _ngl       = ngl;
    _nCtx      = n_ctx;
    _minP      = minP;
    _temp      = temp;
    _topK      = topK;
    _topP      = topP;
    _nThreads = nThreads;
}

void LlamaOperator::setSystemMessage(const std::string& systemMsg) {
    _systemMessage = systemMsg;
}

bool LlamaOperator::loadEmbedModel(const std::string & modelPath, const enum llama_pooling_type poolingType) {
    if (_modelLoaded) {
        WARNING("Embedding Model already loaded");
        return false;
    }

    // Initialize llama backend
    llama_backend_init();

    // Set up logging
    llama_log_set([](enum ggml_log_level level, const char * text, void * /* user_data */) {
        if (level >= GGML_LOG_LEVEL_ERROR) {
            //ERROR("Error: {}", text);
        }
    }, nullptr);

    // Load model
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = _ngl;

    _model = llama_model_load_from_file(modelPath.c_str(), model_params);
    if (!_model) {
        ERROR("Error: Unable to load model");
        return false;
    }

    // Get vocab
    _vocab = llama_model_get_vocab(_model);

    // Set up context parameters
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = _nCtx;
    ctx_params.n_batch = _nCtx;
    ctx_params.n_ubatch = _nCtx;  // For embeddings, ubatch should equal batch
    ctx_params.pooling_type = poolingType;
    ctx_params.embeddings = true;
    ctx_params.n_threads = _nThreads;

    _ctx = llama_init_from_model(_model, ctx_params);
    if (!_ctx) {
        ERROR("Error: Failed to create llama_context");
        llama_model_free(_model);
        return false;
    }

    // Check if model supports embeddings
    if (llama_model_has_encoder(_model) && llama_model_has_decoder(_model)) {
        ERROR("Error: computing embeddings in encoder-decoder models is not supported");
        llama_model_free(_model);
        return false;
    }

    // Initialize sampler (optional for embeddings but kept for compatibility)
    _smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(_smpl, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

    _modelLoaded = true;
    return true;
}

bool LlamaOperator::loadChatModel(const std::string & modelPath) {
    if (_modelLoaded) {
        WARNING("Chat Model already loaded");
        return false;
    }

    llama_log_set([](enum ggml_log_level level, const char * text, void * /* user_data */) {
        if (level >= GGML_LOG_LEVEL_ERROR) {
           // ERROR("Error: {}", text);
        }
    }, nullptr);

    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = _ngl;
    _model = llama_model_load_from_file(modelPath.c_str(), model_params);
    if (!_model) {
        ERROR("Error: Unable to load model");
        return false;
    }
    _vocab = llama_model_get_vocab(_model);
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = _nCtx;
    ctx_params.n_batch = _nCtx;
    ctx_params.n_threads = _nThreads;
    _ctx = llama_init_from_model(_model, ctx_params);

    if (!_ctx) {
        ERROR("Error: Failed to create llama_context");
        return false;
    }
    _smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
    // llama_sampler_chain_add(_smpl, llama_sampler_init_min_p(_minP, 1));
    llama_sampler_chain_add(_smpl, llama_sampler_init_temp(_temp));
    // llama_sampler_chain_add(_smpl, llama_sampler_init_top_k(_topK));
    // llama_sampler_chain_add(_smpl, llama_sampler_init_top_p(_topP, 1));

    llama_sampler_chain_add(_smpl, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

    _modelLoaded = true;
    return true;
}


void LlamaOperator::setCallBackFunction(std::function<void(const std::string &)> func) {
    _responseCallbackFunction = func;
}


std::string LlamaOperator::generateResponse(const std::string& prompt) {
    std::string response;

    const bool is_first = llama_memory_seq_pos_max(llama_get_memory(_ctx), 0) == -1;

    // tokenize the prompt
    const int n_prompt_tokens = -llama_tokenize(_vocab, prompt.c_str(), prompt.size(), NULL, 0, is_first, true);
    std::vector<llama_token> prompt_tokens(n_prompt_tokens);
    if (llama_tokenize(_vocab, prompt.c_str(), prompt.size(), prompt_tokens.data(), prompt_tokens.size(), is_first, true) < 0) {
        GGML_ABORT("failed to tokenize the prompt\n");
    }


    // prepare a batch for the prompt
    llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());
    llama_token new_token_id;
    while (_running) {
        // check if we have enough space in the context to evaluate this batch
        int n_ctx = llama_n_ctx(_ctx);
        int n_ctx_used = llama_memory_seq_pos_max(llama_get_memory(_ctx), 0) + 1;
        if (n_ctx_used + batch.n_tokens > n_ctx) {
            //printf("\033[0m\n");
            fprintf(stderr, "context size exceeded\n");
            exit(0);
        }

        if (llama_decode(_ctx, batch)) {
            GGML_ABORT("failed to decode\n");
        }

        // sample the next token
        new_token_id = llama_sampler_sample(_smpl, _ctx, -1);

        // is it an end of generation?
        if (llama_vocab_is_eog(_vocab, new_token_id)) {
            break;
        }

        // convert the token to a string, print it and add it to the response
        char buf[256];
        int n = llama_token_to_piece(_vocab, new_token_id, buf, sizeof(buf), 0, true);
        if (n < 0) {
            GGML_ABORT("failed to convert token to piece\n");
        }
        _responseCallbackFunction(std::string(buf, n));
        batch = llama_batch_get_one(&new_token_id, 1);
    }

    _responseCallbackFunction("<end>");

    return response;
}


void LlamaOperator::chat(const std::string &userInput) {
    std::vector<char> formatted(llama_n_ctx(_ctx));
    int prev_len = 0;
    _running.store(true);
    if (userInput.empty()) return;

    // İlk mesajsa ve system mesajı varsa, onu ekle
    if (_messages.empty() && !_systemMessage.empty()) {
        _messages.push_back({"system", strdup(_systemMessage.c_str())});
    }

    _messages.push_back({"user", strdup(userInput.c_str())});
    const char* tmpl = llama_model_chat_template(_model, nullptr);
    int new_len = llama_chat_apply_template(tmpl, _messages.data(), _messages.size(), true, formatted.data(), formatted.size());
    std::string prompt(formatted.begin() + prev_len, formatted.begin() + new_len);
    _messages.push_back({"assistant", strdup(generateResponse(prompt).c_str())});

    prev_len = new_len;
}

void LlamaOperator::batch_add_seq(llama_batch & batch, const std::vector<int32_t> & tokens, llama_seq_id seq_id) {
    size_t n_tokens = tokens.size();
    for (size_t i = 0; i < n_tokens; i++) {
        common_batch_add(batch, tokens[i], i, { seq_id }, true);
    }
}


void LlamaOperator::batch_decode(llama_context * ctx, llama_batch & batch, float * output, int n_seq, int n_embd, int embd_norm) {
    const struct llama_model * model = llama_get_model(ctx);
    const enum llama_pooling_type pooling_type = llama_pooling_type(ctx);

    // Run model
    if (llama_model_has_encoder(model) && !llama_model_has_decoder(model)) {
        // encoder-only model
        if (llama_encode(ctx, batch) < 0) {
            ERROR("Error: failed llama_encode");
            return;
        }
    } else if (!llama_model_has_encoder(model) && llama_model_has_decoder(model)) {
        // decoder-only model
        if (llama_decode(ctx, batch) < 0) {
            ERROR("Error: failed llama_decode");
            return;
        }
    }

    // Extract embeddings
    for (int i = 0; i < batch.n_tokens; i++) {
        if (!batch.logits[i]) {
            continue;
        }

        const float * embd = nullptr;
        int embd_pos = 0;

        if (pooling_type == LLAMA_POOLING_TYPE_NONE) {
            // Get token embeddings
            embd = llama_get_embeddings_ith(ctx, i);
            embd_pos = i;
            if (!embd) {
                ERROR("Failed to get token embeddings");
                return;
            }
        } else {
            // Get sequence embeddings - supported only when pooling_type is not NONE
            embd = llama_get_embeddings_seq(ctx, batch.seq_id[i][0]);
            embd_pos = batch.seq_id[i][0];
            if (!embd) {
                ERROR("Failed to get sequence embeddings");
                return;
            }
        }

        float * out = output + embd_pos * n_embd;
        common_embd_normalize(embd, out, n_embd, embd_norm);
    }
}

void LlamaOperator::unloadEmbedModel() {
    if (_smpl) {
        llama_sampler_free(_smpl);
        _smpl = nullptr;
    }

    if (_model) {
        llama_model_free(_model);
        _model = nullptr;
    }
    _vocab = nullptr;
    _modelLoaded = false;
    llama_backend_free();
}

std::vector<float> LlamaOperator::calculateEmbeddings(const std::string& text) {
    if (!_modelLoaded || !_ctx || !_model) {
        ERROR("Model not loaded");
        return std::vector<float>();
    }

    // Tokenize the text
    auto tokens = common_tokenize(_ctx, text, true, true);

    // Check if tokens exceed batch size
    const uint64_t batch_size = _nCtx;
    if (tokens.size() > batch_size) {
        ERROR("Number of tokens ({}) exceeds batch size ({})", tokens.size(), batch_size);
        return std::vector<float>();
    }

    // Check if the last token is SEP (optional warning)
    if (!tokens.empty() && tokens.back() != llama_vocab_sep(_vocab)) {
        // WARNING("Last token in the prompt is not SEP");
    }

    // Initialize batch
    llama_batch batch = llama_batch_init(batch_size, 0, 1);

    // Clear any previous KV cache
    llama_memory_clear(llama_get_memory(_ctx), true);

    // Add tokens to batch
    batch_add_seq(batch, tokens, 0);

    // Get embedding dimensions
    const int embedding_dim = llama_model_n_embd(_model);
    const enum llama_pooling_type pooling_type = llama_pooling_type(_ctx);

    // Calculate number of embeddings we'll get
    int n_embd_count = 1;  // For sequence pooling, we get one embedding per sequence
    if (pooling_type == LLAMA_POOLING_TYPE_NONE) {
        n_embd_count = tokens.size();  // For token pooling, we get one embedding per token
    }

    // Prepare output embeddings
    std::vector<float> embeddings(n_embd_count * embedding_dim, 0.0f);

    // Decode and get embeddings
    batch_decode(_ctx, batch, embeddings.data(), 1, embedding_dim, 2);

    // Cleanup
    llama_batch_free(batch);

    return embeddings;
}

void LlamaOperator::unloadModel() {
    for (auto &msg: _messages) {
        free(const_cast<char *>(msg.content));
    }
    llama_sampler_free(_smpl);
    llama_free(_ctx);
    llama_model_free(_model);
}

void LlamaOperator::stopGenerateResponse() {
    _running.store(false);
}
