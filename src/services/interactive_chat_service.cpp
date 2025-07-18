#include "interactive_chat_service.h"
#include "gesture_performing_service.h"
#include "remote_connection_service.h"
#include "tui_service.h"
#include "../logger.h"
#include <regex>


InteractiveChatService* InteractiveChatService::_instance = nullptr;

InteractiveChatService *InteractiveChatService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new InteractiveChatService();
    }
    return _instance;
}


InteractiveChatService::InteractiveChatService() : Service("InteractiveChatService") {

}


InteractiveChatService::~InteractiveChatService()
{
}

bool InteractiveChatService::send_query(const std::string& message){
    INFO("Llama Query is started!");
    _llamaChatOperator.chat(message);

    return true;

}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";

    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

void InteractiveChatService::query_response_callback(const std::string &response) {
    static std::string accumulated_text;

    if (response == "<end>") {
        _queryRunning = false;
        INFO("Llama Query finished!");
        accumulated_text.clear();
        return;
    }

    accumulated_text += response;
    std::regex sentence_end_regex(R"(([.!?]+)(\s+|$))");
    std::smatch match;

    std::string remaining_text = accumulated_text;
    size_t last_processed_pos = 0;

    while (std::regex_search(remaining_text, match, sentence_end_regex)) {
        size_t sentence_end_pos = match.position() + match[1].length();
        std::string complete_sentence = remaining_text.substr(0, sentence_end_pos);

        // Cümleyi temizle
        std::string trimmed_sentence = trim(complete_sentence);

        if (trimmed_sentence.length() >= 4) {
            std::unique_ptr<MessageData> data = std::make_unique<LLMResponseData>();
            static_cast<LLMResponseData*>(data.get())->sentence = trimmed_sentence;

            std::pair<EmotionType,float> emotion = find_sentence_emotion(trimmed_sentence);
            static_cast<LLMResponseData*>(data.get())->emotion = emotion.first;
            static_cast<LLMResponseData*>(data.get())->emotionSimilarity = emotion.second;

            std::pair<ReactionType,float> reaction = find_sentence_reaction(trimmed_sentence);
            static_cast<LLMResponseData*>(data.get())->reaction = reaction.first;
            static_cast<LLMResponseData*>(data.get())->reactionSimilarity = reaction.second;
            publish(MessageType::LLMResponse, data);

            // Kalan metni güncelle
            remaining_text = remaining_text.substr(sentence_end_pos);
            if (match[2].length() > 0) {
                remaining_text = remaining_text.substr(match[2].length() - 1);
            }

            last_processed_pos = sentence_end_pos;
        } else {
            // Cümle kısa olduğu için parçayı işlemeye devam et (accumulate etmeye devam)
            break;
        }
    }

    accumulated_text = remaining_text;
}


bool InteractiveChatService::load_models()
{
    _llamaChatOperator.setCallBackFunction( std::bind(&InteractiveChatService::query_response_callback, this, std::placeholders::_1));

    INFO("Llama Chat Model is loading...");
    bool ret = _llamaChatOperator.loadChatModel();
    if (!ret) {
        ERROR("Llama Chat Model loading failed!");
    }
    DEBUG("Llama Chat Model is loaded");

    INFO("Llama Embedding Model is loading...");
    ret = _llamaEmbeddingOperator.loadEmbedModel();
    if (!ret) {
        ERROR("Llama Embedding Model loading failed!");
    }
    DEBUG("Llama Embedding Model is loaded");

    return ret;
}

void InteractiveChatService::calculate_embeddings() {
    INFO("Calculating Emotional Embeddings...");
    for (auto& emotion : Emotional_Gesture_List) {
        emotion.embedding = _llamaEmbeddingOperator.calculateEmbeddings(emotion.description);
    }

    INFO("Calculating Reactional Embeddings...");
    for (auto &reaction : Reactional_Gesture_List) {
        reaction.embedding = _llamaEmbeddingOperator.calculateEmbeddings(reaction.description);
    }

    INFO("Calculating Directive Embeddings...");
    for (auto &directive : Directive_List) {
        directive.embedding = _llamaEmbeddingOperator.calculateEmbeddings(directive.description);
    }
}


std::pair<EmotionType,float> InteractiveChatService::find_sentence_emotion(const std::string &sentence) {
    float max_similarity = 0.0f;
    EmotionType best_match;
    std::vector<float> sentence_embeddings = _llamaEmbeddingOperator.calculateEmbeddings(sentence);

    for (auto emotion : Emotional_Gesture_List) {

        float sim = _llamaEmbeddingOperator.getSimilarity(sentence_embeddings, emotion.embedding);

        if (sim > max_similarity && sim > 0.5f ) {
            max_similarity = sim;
            best_match = emotion.emotion;
        }
    }

    return std::make_pair(best_match, max_similarity);
}

std::pair<ReactionType, float> InteractiveChatService::find_sentence_reaction(const std::string &sentence) {
    float max_similarity = 0.0f;
    ReactionType best_match;
    std::vector<float> sentence_embeddings = _llamaEmbeddingOperator.calculateEmbeddings(sentence);

    for (auto reaction : Reactional_Gesture_List) {

        float sim = _llamaEmbeddingOperator.getSimilarity(sentence_embeddings, reaction.embedding);

        if (sim > max_similarity && sim > 0.5f ) {
            max_similarity = sim;
            best_match = reaction.reaction;
        }
    }

    return std::make_pair(best_match, max_similarity);
}

void InteractiveChatService::service_function()
{
    load_models();
    calculate_embeddings();

    subscribe_to_service(TuiService::get_instance());
    subscribe_to_service(RemoteConnectionService::get_instance());
    subscribe_to_service(GesturePerformingService::get_instance());

}

void InteractiveChatService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) {
    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::LLMQuery: {
            if (data) {
                std::string queryMsg = static_cast<LLMQueryData*>(data.get())->query;
                llm_query(queryMsg);
            }
            break;
        }
        default:
            WARNING("{} subcribed_data_receive unknown message type!", get_service_name());
            break;
    }
}

void InteractiveChatService::llm_query(const std::string &query) {
    if (_queryRunning) {
        WARNING("llama Query is already running!");
    }else {
        _queryRunning = true;
        send_query(query);
    }

}

