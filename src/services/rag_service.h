#ifndef RAG_SERVICE_H
#define RAG_SERVICE_H

#include <atomic>
#include <string>
#include <nlohmann/json.hpp>
#include "service.h"
#include "../operators/llama_operator.h"
#include "../public_data_messages.h"
#include "../gesture_defs.h"
#include "../operators/json_parser_operator.h"

class RagService : public Service{
public:

    static RagService *get_instance();
    virtual ~RagService();

private:
    static RagService *_instance;
    std::atomic<bool> _queryRunning{false};
    std::mutex _quryMutex;
    std::condition_variable _queryCondition;
    std::vector<std::vector<float>> _ragEmbeddings;

    LlamaOperator _embeddingOperator;

    std::list<EmotionalGesture> _emotionalList;
    std::list<ReactionalGesture> _reactionalList;
    std::list<Directive> _directiveList;

    RagService();

    bool initialize();
    void service_function();
    void updateRAGDatabase();
    void showRAGDatabase();
    void clearRAGDatabase();
    std::string getOutputForId(int id);
    std::string getRAGResponse(const std::string &query);
    void divideAndPublish(const std::string &response);
    std::vector<std::string> splitIntoSentences(const std::string& paragraph);
    void calculate_gesture_embeddings();
    std::pair<EmotionalGesture, float> find_sentence_emotion(const std::string& sentence);
    std::pair<ReactionalGesture, float> find_sentence_reaction(const std::string& sentence);
    std::pair<Directive, float> find_sentence_directive(const std::string &sentence);
    void speak(std::string text);

    void loadDatabase();

    //subscribed data functions
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);

};



#endif //RAG_SERVICE_H
