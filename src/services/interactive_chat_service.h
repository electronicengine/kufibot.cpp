#ifndef INTERACTIVE_CHAT_SERVICE_H
#define INTERACTIVE_CHAT_SERVICE_H


#include <atomic>
#include <string>
#include <nlohmann/json.hpp>
#include "service.h"
#include "../operators/llama_operator.h"
#include "../public_data_messages.h"
#include "../gesture_defs.h"


struct LlamaOptions {
    std::string llamaChatModelPath;
    std::string llamaEmbeddingModelPath;
    double temperature;
    int maxTokenSize;
    double topK;
    double topP;
    int nThreads;
    int poolingType;
};


class InteractiveChatService : public Service {

public:

    static InteractiveChatService *get_instance();
    virtual ~InteractiveChatService();
    void setLlamaChatOptions(const LlamaOptions& options){ _llamaChatOptions = options;}
    void setLlamaEmebddingOptions(const LlamaOptions& options){_llamaEmbeddingOptions = options;}

private:
    static InteractiveChatService *_instance;
    std::atomic<bool> _queryRunning{false};
    std::mutex _quryMutex;
    std::condition_variable _queryCondition;

    LlamaOptions _llamaChatOptions;
    LlamaOptions _llamaEmbeddingOptions;

    LlamaOperator _llamaChatOperator;
    LlamaOperator _llamaEmbeddingOperator;

    std::list<EmotionalGesture> _emotionalList;
    std::list<ReactionalGesture> _reactionalList;
    std::list<Directive> _directiveList;

    InteractiveChatService();

    bool send_query(const std::string& message);
    void query_response_callback(const std::string& response);
    bool load_models();
    void calculate_embeddings();
    std::pair<EmotionalGesture, float> find_sentence_emotion(const std::string& sentence);
    std::pair<ReactionalGesture, float> find_sentence_reaction(const std::string& sentence);
    std::pair<Directive, float> find_sentence_directive(const std::string &sentence);

    void service_function();

    //subscribed data functions
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);

    void llm_query(const std::string &query);

};

#endif