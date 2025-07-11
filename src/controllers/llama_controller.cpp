// LlamaController.cpp
#include "llama_controller.h"
#include <iostream>
#include <cstring>


LlamaController::LlamaController()
{
    _ngl = 99;
    _nCtx = 2048;
    _minP = 0.05f;
    _temp = 0.8f;
    _topK = 50;
    _topP = 0.9;
    _model = nullptr;
    _ctx = nullptr;
    _smpl = nullptr;
    _vocab = nullptr;
}

LlamaController::~LlamaController() {
    freeResources();
}

void LlamaController::setOptions(int ngl, int nThreads, int n_ctx, float minP, float temp, int topK,
                               float topP) {
    _ngl       = ngl;
    _nCtx      = n_ctx;
    _minP      = minP;
    _temp      = temp;
    _topK      = topK;
    _topP      = topP;
    _nThreads = nThreads;
}

void LlamaController::loadEmbedModel(const std::string & modelPath, const enum llama_pooling_type poolingType) {

    llama_log_set([](enum ggml_log_level level, const char * text, void * /* user_data */) {
        if (level >= GGML_LOG_LEVEL_ERROR) {
            fprintf(stderr, "%s", text);
        }
    }, nullptr);

    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = _ngl;
    _model = llama_model_load_from_file(modelPath.c_str(), model_params);
    if (!_model) {
        throw std::runtime_error("Error: Unable to load model");
    }
    _vocab = llama_model_get_vocab(_model);
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = _nCtx;
    ctx_params.n_batch = _nCtx;
    ctx_params.pooling_type = poolingType;
    ctx_params.embeddings = true;
    ctx_params.n_threads = _nThreads;

    _ctx = llama_init_from_model(_model, ctx_params);
    if (!_ctx) {
        throw std::runtime_error("Error: Failed to create llama_context");
    }
    _smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(_smpl, llama_sampler_init_min_p(_minP, 1));
    llama_sampler_chain_add(_smpl, llama_sampler_init_temp(_temp));
    llama_sampler_chain_add(_smpl, llama_sampler_init_top_k(_topK));
    llama_sampler_chain_add(_smpl, llama_sampler_init_top_p(_topP, 1));

    llama_sampler_chain_add(_smpl, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
}

bool LlamaController::loadChatModel(const std::string & modelPath) {

    llama_log_set([](enum ggml_log_level level, const char * text, void * /* user_data */) {
        if (level >= GGML_LOG_LEVEL_ERROR) {
            fprintf(stderr, "%s", text);
        }
    }, nullptr);

    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = _ngl;
    _model = llama_model_load_from_file(modelPath.c_str(), model_params);
    if (!_model) {
        //throw std::runtime_error("Error: Unable to load model");
        return false;
    }
    _vocab = llama_model_get_vocab(_model);
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = _nCtx;
    ctx_params.n_batch = _nCtx;
    ctx_params.n_threads = _nThreads;
    _ctx = llama_init_from_model(_model, ctx_params);
    if (!_ctx) {
        //throw std::runtime_error("Error: Failed to create llama_context");
        return false;
    }
    _smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(_smpl, llama_sampler_init_min_p(_minP, 1));
    llama_sampler_chain_add(_smpl, llama_sampler_init_temp(_temp));
    llama_sampler_chain_add(_smpl, llama_sampler_init_top_k(_topK));
    llama_sampler_chain_add(_smpl, llama_sampler_init_top_p(_topP, 1));

    llama_sampler_chain_add(_smpl, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

    return true;
}

void LlamaController:: setCallBackFunction(std::function<void(const std::string &)> func) {
    _responseCallbackFunction = func;
}


std::string LlamaController::generateResponse(const std::string& prompt) {
    std::string response;
    // We can simply set is_first to true since we're starting a new generation
    const bool is_first = true;
    const int n_prompt_tokens = -llama_tokenize(_vocab, prompt.c_str(), prompt.size(), NULL, 0, is_first, true);
    std::vector<llama_token> prompt_tokens(n_prompt_tokens);
    llama_tokenize(_vocab, prompt.c_str(), prompt.size(), prompt_tokens.data(), prompt_tokens.size(), is_first, true);
    llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());
    llama_token new_token_id;
    while (true) {
        if (llama_decode(_ctx, batch)) {
            throw std::runtime_error("Error: Failed to decode");
        }
        new_token_id = llama_sampler_sample(_smpl, _ctx, -1);
        if (llama_vocab_is_eog(_vocab, new_token_id)) {
            break;
        }
        char buf[256];
        int n = llama_token_to_piece(_vocab, new_token_id, buf, sizeof(buf), 0, true);
        if(_responseCallbackFunction)
            _responseCallbackFunction(std::string(buf, n));

        response += std::string(buf, n);
        batch = llama_batch_get_one(&new_token_id, 1);
    }
    if(_responseCallbackFunction)
        _responseCallbackFunction("<end>");

    return response;
}


void LlamaController::chat(const std::string &userInput) {
    std::vector<char> formatted(llama_n_ctx(_ctx));
    int prev_len = 0;

    if (userInput.empty()) return;
    _messages.push_back({"user", strdup(userInput.c_str())});
    const char* tmpl = llama_model_chat_template(_model, nullptr);
    int new_len = llama_chat_apply_template(tmpl, _messages.data(), _messages.size(), true, formatted.data(), formatted.size());
    std::string prompt(formatted.begin() + prev_len, formatted.begin() + new_len);
    _messages.push_back({"assistant", strdup(generateResponse(prompt).c_str())});
    prev_len = new_len;

}

void LlamaController::batch_add_seq(llama_batch & batch, const std::vector<int32_t> & tokens, llama_seq_id seq_id)
{
    size_t n_tokens = tokens.size();
    for (size_t i = 0; i < n_tokens; i++) {
        common_batch_add(batch, tokens[i], i, { seq_id }, true);
    }
}


void LlamaController::batch_decode(llama_context * ctx, llama_batch & batch, float * output, int n_seq, int n_embd, int embd_norm) {
    const struct llama_model * model = llama_get_model(ctx);
    const enum llama_pooling_type pooling_type = llama_pooling_type(ctx);

    // run model
    if (llama_model_has_encoder(model) && !llama_model_has_decoder(model)) {
        // encoder-only model
        if (llama_encode(ctx, batch) < 0) {
            std::cout << "error" << std::endl;
        }
    } else if (!llama_model_has_encoder(model) && llama_model_has_decoder(model)) {
        // decoder-only model
        int32_t ret = llama_decode(ctx, batch);

        if (ret < 0) {
            std::cout << "error" << std::endl;
        }
    }

    for (int i = 0; i < batch.n_tokens; i++) {
        if (!batch.logits[i]) {
            continue;
        }

        const float * embd = nullptr;
        int embd_pos = 0;

        if (pooling_type == LLAMA_POOLING_TYPE_NONE) {
            // try to get token embeddings
            embd = llama_get_embeddings_ith(ctx, i);
            embd_pos = i;
            GGML_ASSERT(embd != NULL && "failed to get token embeddings");
        } else {
            // try to get sequence embeddings - supported only when pooling_type is not NONE
            auto seq_id = batch.seq_id[i][0];

            embd = llama_get_embeddings_seq(ctx, seq_id);
            embd_pos = batch.seq_id[i][0];
            GGML_ASSERT(embd != NULL && "failed to get sequence embeddings");
        }

        float * out = output + embd_pos * n_embd;
        common_embd_normalize(embd, out, n_embd, embd_norm);
    }
}


std::vector<float> LlamaController::calculateEmbeddings(const std::string& text) {
    
    uint64_t batch_size = 2048;

    auto tokens = common_tokenize(_ctx, text, true, true);

    // Initialize batch
    llama_batch batch = llama_batch_init(batch_size, 0, 1);

    // Prepare output embeddings
    const int embedding_dim = llama_model_n_embd(_model);
    std::vector<float> embeddings(embedding_dim, 0);

    // Process tokens in batch
    batch_add_seq(batch, tokens, 0);  // Add all tokens at once (simplified)

    // Decode and get embeddings
    batch_decode(_ctx, batch, embeddings.data(), 1, embedding_dim, 2);

    // Cleanup
    llama_batch_free(batch);

    return embeddings;
}

void LlamaController::freeResources() {
    for (auto& msg : _messages) {
        free(const_cast<char*>(msg.content));
    }
    llama_sampler_free(_smpl);
    llama_free(_ctx);
    llama_model_free(_model);
}