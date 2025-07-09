//
// Created by ulak on 25.06.2025.
//

#ifndef LLAMACONTROLLER_H
#define LLAMACONTROLLER_H

#include "llama.h"
#include "common.h"
#include <string>
#include <vector>
#include <iostream>
#include <functional>


class LlamaController {
public:
    LlamaController();
    ~LlamaController();

    void batch_add_seq(llama_batch & batch, const std::vector<int32_t> & tokens, llama_seq_id seq_id);
    void batch_decode(llama_context * ctx, llama_batch & batch, float * output, int n_seq, int n_embd, int embd_norm);

    void setOptions(int ngl = 99, int nThreads = 4, int n_ctx = 2048, float minP = 0.05f, float temp = 0.8f, int topK = 50, float topP = 0.9);
    void loadEmbedModel(const std::string & modelPath, const enum llama_pooling_type poolingType = llama_pooling_type::LLAMA_POOLING_TYPE_MEAN);
    void loadChatModel(const std::string& modelPath);
    void setCallBackFunction(std::function<void(const std::string&)> func);
    std::string generateResponse(const std::string& prompt);
    void chat(const std::string &userInput);
    std::vector<float> calculateEmbeddings(const std::string& text);
    float getSimilarity(const std::vector<float>& Emb1, const std::vector<float>& Emb2){ return common_embd_similarity_cos(&Emb1[0], &Emb2[0], Emb1.size()); }

private:
    void freeResources();
    int _nThreads;
    int _ngl;
    int _nCtx;
    float _minP;
    float _temp;
    int _topK;
    float _topP;

    llama_model* _model;
    llama_context* _ctx;
    llama_sampler* _smpl;
    const llama_vocab* _vocab;
    std::vector<llama_chat_message> _messages;
    std::function<void(const std::string&)> _responseCallbackFunction;

};


#endif //LLAMACONTROLLER_H
