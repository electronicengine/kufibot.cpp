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

#include "interactive_chat_service.h"
#include "gesture_performer_service.h"
#include "remote_connection_service.h"
#include "tui_service.h"
#include "../logger.h"
#include <regex>

#include "gesture_recognizer_service.h"
#include "landmark_tracker_service.h"
#include "../operators/json_parser_operator.h"
#include "../operators/speech_recognizing_operator.h"


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
        _queryCondition.notify_all();
        INFO("Llama Query finished!");

        // Process any remaining text before clearing
        if (!accumulated_text.empty()) {
            std::string trimmed_sentence = trim(accumulated_text);
            if (trimmed_sentence.length() >= 4) {
                std::unique_ptr<MessageData> data = std::make_unique<LLMResponseData>();
                static_cast<LLMResponseData *>(data.get())->sentence = trimmed_sentence;
                static_cast<LLMResponseData *>(data.get())->endMarker = true;
                std::pair<EmotionalGesture, float> emotion = find_sentence_emotion(trimmed_sentence);
                static_cast<LLMResponseData *>(data.get())->emotionalGesture = emotion.first;
                static_cast<LLMResponseData *>(data.get())->emotionSimilarity = emotion.second;

                std::pair<ReactionalGesture, float> reaction = find_sentence_reaction(trimmed_sentence);
                static_cast<LLMResponseData *>(data.get())->reactionalGesture = reaction.first;
                static_cast<LLMResponseData *>(data.get())->reactionSimilarity = reaction.second;

                publish(MessageType::LLMResponse, data);
            }
        }else {
            std::unique_ptr<MessageData> data = std::make_unique<LLMResponseData>();
            static_cast<LLMResponseData *>(data.get())->sentence = "";
            static_cast<LLMResponseData *>(data.get())->endMarker = true;

            publish(MessageType::LLMResponse, data);
        }
        accumulated_text.clear();

        return;
    }

    accumulated_text += response;
    std::regex sentence_end_regex(R"(([.!?]+)(\s+|$))");
    std::smatch match;

    std::string remaining_text = accumulated_text;

    while (std::regex_search(remaining_text, match, sentence_end_regex)) {
        size_t sentence_end_pos = match.position() + match[1].length();
        std::string complete_sentence = remaining_text.substr(0, sentence_end_pos);

        // Cümleyi temizle
        std::string trimmed_sentence = trim(complete_sentence);

        if (trimmed_sentence.length() >= 4) {
            std::unique_ptr<MessageData> data = std::make_unique<LLMResponseData>();
            static_cast<LLMResponseData*>(data.get())->sentence = trimmed_sentence;
            static_cast<LLMResponseData *>(data.get())->endMarker = false;

            std::pair<EmotionalGesture,float> emotion = find_sentence_emotion(trimmed_sentence);
            static_cast<LLMResponseData*>(data.get())->emotionalGesture = emotion.first;
            static_cast<LLMResponseData*>(data.get())->emotionSimilarity = emotion.second;

            std::pair<ReactionalGesture,float> reaction = find_sentence_reaction(trimmed_sentence);
            static_cast<LLMResponseData*>(data.get())->reactionalGesture = reaction.first;
            static_cast<LLMResponseData*>(data.get())->reactionSimilarity = reaction.second;

            publish(MessageType::LLMResponse, data);

            // Kalan metni güncelle
            remaining_text = remaining_text.substr(sentence_end_pos);
            if (match[2].length() > 0) {
                remaining_text = remaining_text.substr(match[2].length() - 1);
            }

        } else {
            break;
        }
    }

    accumulated_text = remaining_text;
}


bool InteractiveChatService::load_models()
{
    auto parser = JsonParserOperator::get_instance();
    auto aiConfig = parser->getAiConfig();
    if (!aiConfig.has_value()) {
        ERROR("AI Config is not found!");
        return false;
    }

    _llamaChatOperator.setCallBackFunction( std::bind(&InteractiveChatService::query_response_callback, this, std::placeholders::_1));
    _llamaChatOperator.setSystemMessage(aiConfig->llmChatConfig.systemMessage);


    INFO("Llama Chat Model is loading...");
    bool ret = _llamaChatOperator.loadChatModel(aiConfig->llmChatConfig.modelPath);
    if (!ret) {
        ERROR("Llama Chat Model loading failed!");
        return false;
    }
    DEBUG("Llama Chat Model is loaded");

    INFO("Llama Embedding Model is loading...");
    ret = _llamaEmbeddingOperator.loadEmbedModel(aiConfig->llmEmbeddingConfig.modelPath, (enum llama_pooling_type) aiConfig->llmEmbeddingConfig.poolingType);
    if (!ret) {
        ERROR("Llama Embedding Model loading failed!");
        return false;
    }
    DEBUG("Llama Embedding Model is loaded");

    return ret;
}

void InteractiveChatService::calculate_embeddings() {
    INFO("Calculating Emotional Embeddings...");
    for (auto& emotion : _emotionalList) {
        emotion.embedding = _llamaEmbeddingOperator.calculateEmbeddings(emotion.description);
    }

    INFO("Calculating Reactional Embeddings...");
    for (auto &reaction : _reactionalList) {
        reaction.embedding = _llamaEmbeddingOperator.calculateEmbeddings(reaction.description);
    }

    INFO("Calculating Directive Embeddings...");
    for (auto &directive : _directiveList) {
        directive.embedding = _llamaEmbeddingOperator.calculateEmbeddings(directive.description);
    }
}


std::pair<EmotionalGesture,float> InteractiveChatService::find_sentence_emotion(const std::string &sentence) {
    float max_similarity = 0.0f;
    EmotionalGesture best_match;
    std::vector<float> sentence_embeddings = _llamaEmbeddingOperator.calculateEmbeddings(sentence);

    for (auto emotion : _emotionalList) {

        float sim = _llamaEmbeddingOperator.getSimilarity(sentence_embeddings, emotion.embedding);

        if (sim > max_similarity && sim > 0.5f ) {
            max_similarity = sim;
            best_match = emotion;
        }
    }

    return std::make_pair(best_match, max_similarity);
}

std::pair<ReactionalGesture, float> InteractiveChatService::find_sentence_reaction(const std::string &sentence) {
    float max_similarity = 0.0f;
    ReactionalGesture best_match;
    std::vector<float> sentence_embeddings = _llamaEmbeddingOperator.calculateEmbeddings(sentence);

    for (auto reaction : _reactionalList) {
        float sim = _llamaEmbeddingOperator.getSimilarity(sentence_embeddings, reaction.embedding);

        if (sim > max_similarity && sim > 0.5f ) {
            max_similarity = sim;
            best_match = reaction;
        }
    }

    return std::make_pair(best_match, max_similarity);
}

std::pair<Directive, float> InteractiveChatService::find_sentence_directive(const std::string &sentence) {
    float max_similarity = 0.0f;
    Directive best_match;
    std::vector<float> sentence_embeddings = _llamaEmbeddingOperator.calculateEmbeddings(sentence);

    for (auto directive : _directiveList) {
        float sim = _llamaEmbeddingOperator.getSimilarity(sentence_embeddings, directive.embedding);

        if (sim > max_similarity && sim > 0.5f ) {
            max_similarity = sim;
            best_match = directive;
        }
    }

    return std::make_pair(best_match, max_similarity);
}

bool InteractiveChatService::initialize() {
    auto parser = JsonParserOperator::get_instance();
    if (parser->getEmotionalList().has_value()) {
        _emotionalList = parser->getEmotionalList().value();
    }else {
        ERROR("Emotional List is not found!");
        return false;
    }

    if (parser->getReactionalList().has_value()) {
        _reactionalList = parser->getReactionalList().value();
    }else {
        ERROR("Reactional List is not found!");
        return false;
    }

    if (parser->getDirectiveList().has_value()) {
        _directiveList = parser->getDirectiveList().value();
    }else {
        ERROR("Directive List is not found!");
        return false;
    }

    auto ret = load_models();
    if (!ret) {
        return false;
    }

    calculate_embeddings();

    subscribe_to_service(TuiService::get_instance());
    subscribe_to_service(RemoteConnectionService::get_instance());
    subscribe_to_service(GesturePerformerService::get_instance());
    subscribe_to_service(LandmarkTrackerService::get_instance());

    INFO("Speech Recognizing Model is loading...");
    auto* recognizer = SpeechRecognizingOperator::get_instance();
    recognizer->load_model(parser->getAiConfig()->speechRecognizerConfig.modelPath);
    recognizer->open();

    return true;
}

void InteractiveChatService::service_function() {

    auto aiConfig = JsonParserOperator::get_instance()->getAiConfig();
    auto *recognizer = SpeechRecognizingOperator::get_instance();
    recognizer->start_listen();

    while (_running) {
        std::string message = recognizer->get_message(3000); // 5 second timeout

        if (!message.empty()) {
            if (message == "<start>") {
                recognizer->stop_listen();
                speak(aiConfig->speechRecognizerConfig.command);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                recognizer->start_listen();
                INFO("Start Listen");
            } else {

                // Process the recognized speech
                json j = json::parse(message);
                std::string text = j["alternatives"][0]["text"];
                INFO("Recognized: {}", text);
                INFO("Stop Listen");
                recognizer->stop_listen();
                std::pair<Directive, float> sim = find_sentence_directive(text);
                INFO("Directive: {}, Sim: {}", sim.first.symbol, sim.second);

                if (sim.second >= 0.8f) {
                    std::unique_ptr<MessageData> data = std::make_unique<LLMResponseData>();
                    static_cast<LLMResponseData *>(data.get())->sentence = sim.first.description;

                    std::pair<Directive, float> directive = sim;
                    static_cast<LLMResponseData *>(data.get())->directive = sim.first;
                    static_cast<LLMResponseData *>(data.get())->directiveSimilarity = sim.second;

                    publish(MessageType::LLMResponse, data);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    recognizer->setListeningMode(false);
                } else {
                    recognizer->setListeningMode(false);
                    llm_query(text);
                    {
                        std::unique_lock<std::mutex> lock(_quryMutex);
                        _queryCondition.wait(lock);
                    }
                }

                INFO("Start Listen");
                recognizer->start_listen();
            }
        }
    }
    recognizer->stop_listen();
}


void InteractiveChatService::speak(std::string text) {
    INFO("Speaking: {}", text);
    std::unique_ptr<MessageData> data = std::make_unique<SpeakRequestData>();
    static_cast<SpeakRequestData *>(data.get())->text = text;
    publish(MessageType::SpeakRequest, data);
}


void InteractiveChatService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data) {

    switch (type) {
        case MessageType::LLMQuery: {
            if (data) {
                std::lock_guard<std::mutex> lock(_dataMutex);

                std::string queryMsg = static_cast<LLMQueryData*>(data.get())->query;
                llm_query(queryMsg);
            }
            break;
        }

        case MessageType::AIModeOnCall : {
            start();
            break;
        }

        case MessageType::AIModeOffCall : {
            _llamaChatOperator.stopGenerateResponse();
            stop();
            break;
        }

        default:
            break;
    }
}

void InteractiveChatService::llm_query(const std::string &query) {
    if (_queryRunning) {
        WARNING("llama Query is already running!");
    }else if (!_running) {
        WARNING("Interactive Chat Service is not running!");
    }else {
        INFO("Llm query: {}", query);
        publish(MessageType::InteractiveChatStarted);

        _queryRunning = true;
        std::thread llmQuery = std::thread([query, this] () {
            _llamaChatOperator.chat(query);
        });
        llmQuery.detach();

    }

}

