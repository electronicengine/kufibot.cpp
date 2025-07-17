//
// Created by ulak on 25.06.2025.
//

#ifndef LLAMA_OPERATOR_H
#define LLAMA_OPERATOR_H


#include "llama.h"
#include "common.h"
#include <string>
#include <vector>
#include <functional>


class LlamaOperator {
public:
    LlamaOperator();
    ~LlamaOperator();

    void batch_add_seq(llama_batch & batch, const std::vector<int32_t> & tokens, llama_seq_id seq_id);
    void batch_decode(llama_context * ctx, llama_batch & batch, float * output, int n_seq, int n_embd, int embd_norm);

    void setOptions(int ngl = 99, int nThreads = 4, int n_ctx = 2048, float minP = 0.05f, float temp = 0.8f, int topK = 50, float topP = 0.9);
    bool loadEmbedModel(const std::string & modelPath ="/usr/ai.models/trLlamaModel/mxbaiV1.gguf",
                            const enum llama_pooling_type poolingType = llama_pooling_type::LLAMA_POOLING_TYPE_MEAN);
    bool loadChatModel(const std::string& modelPath = "/usr/ai.models/trLlamaModel/dolphin3.gguf");
    void unloadModel();

    void setCallBackFunction(std::function<void(const std::string&)> func);
    std::string generateResponse(const std::string& prompt);
    void chat(const std::string &userInput);
    std::vector<float> calculateEmbeddings(const std::string& text);
    float getSimilarity(const std::vector<float>& Emb1, const std::vector<float>& Emb2){ return common_embd_similarity_cos(&Emb1[0], &Emb2[0], Emb1.size()); }
    void unloadEmbedModel();

private:
    int _nThreads;
    int _ngl;
    int _nCtx;
    float _minP;
    float _temp;
    int _topK;
    float _topP;

    bool _modelLoaded;
    llama_model* _model;
    llama_context* _ctx;
    llama_sampler* _smpl;
    llama_batch* _batch;

    const llama_vocab* _vocab;
    std::vector<llama_chat_message> _messages;
    std::function<void(const std::string&)> _responseCallbackFunction;

};


#endif //LLAMA_OPERATOR_H
