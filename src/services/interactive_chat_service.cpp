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


// Yardımcı fonksiyon - string'i temizlemek için
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";

    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

void InteractiveChatService::query_response_callback(const std::string &response) {
    static std::string accumulated_text;

    std::unique_ptr<MessageData> data = std::make_unique<LLMResponseData>();
    static_cast<LLMResponseData*>(data.get())->response = response;
    publish(MessageType::LLMResponse, data);


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
        // Tam cümleyi al (cümle sonu işareti dahil)
        size_t sentence_end_pos = match.position() + match[1].length();
        std::string complete_sentence = remaining_text.substr(0, sentence_end_pos);

        // Cümleyi temizle (başındaki ve sonundaki boşlukları kaldır)
        complete_sentence = trim(complete_sentence);

        if (!complete_sentence.empty()) {
            // Duygu analizi yap
            LlamaResponseEmotion emotion = find_sentence_emotion(complete_sentence);
            std::string emotion_text = " <" + Response_Emotion_Names_EN.at(emotion) + ">";

            // Sonucu yayınla
            std::unique_ptr<MessageData> data = std::make_unique<LLMResponseData>();
            static_cast<LLMResponseData*>(data.get())->response = emotion_text;
            publish(MessageType::LLMResponse, data);
        }

        // Kalan metni güncelle
        remaining_text = remaining_text.substr(sentence_end_pos);
        if (match[2].length() > 0) {
            remaining_text = remaining_text.substr(match[2].length() - 1); // Boşluğu koru
        }

        last_processed_pos = sentence_end_pos;
    }

    // İşlenmemiş kısmı sakla
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

void InteractiveChatService::calculate_emotion_embeddings() {
    INFO("Calculating Emotion Embeddings...");

    for (auto emotion : Response_Emotion_Names_EN) {
        _emotionEmbeddings[emotion.first] = _llamaEmbeddingOperator.calculateEmbeddings(emotion.second);
    }
}

void InteractiveChatService::calculate_directive_embeddigns() {
    INFO("Calculating Directive Embeddings...");

}

LlamaResponseEmotion InteractiveChatService::find_sentence_emotion(const std::string &sentence) {
    float max_similarity = 0.0f;
    LlamaResponseEmotion best_match;

    std::vector<float> sentence_embeddings = _llamaEmbeddingOperator.calculateEmbeddings(sentence);

    for (auto emotion : Response_Emotion_Names_EN) {
        float sim = _llamaEmbeddingOperator.getSimilarity(sentence_embeddings, _emotionEmbeddings[emotion.first]);
        if (max_similarity < sim) {
            max_similarity = sim;
            best_match = emotion.first;
        }
    }

    return best_match;
}

void InteractiveChatService::service_function()
{
    load_models();
    calculate_emotion_embeddings();
    calculate_directive_embeddigns();

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

