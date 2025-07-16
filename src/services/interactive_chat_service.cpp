#include "interactive_chat_service.h"
#include "gesture_performing_service.h"
#include "remote_connection_service.h"
#include "tui_service.h"
#include "../logger.h"


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
    std::lock_guard<std::mutex> lock(_dataMutex);

    _llamaChatOperator.chat(message);

    return true;

}


void InteractiveChatService::query_response_callback(const std::string &response) {
    std::unique_ptr<MessageData> data = std::make_unique<LLMResponseData>();
    static_cast<LLMResponseData*>(data.get())->response = response;
    publish(MessageType::LLMResponse, data);

    if (response == "<end>") {
        _queryRunning = false;
        Logger::info("Llama Query finished!");
    }
}

bool InteractiveChatService::load_model()
{
    _llamaChatOperator.setCallBackFunction([this](const std::string& text) {
        query_response_callback(text);
    });

    Logger::info("Llama Chat Model loading...");
    bool ret = _llamaChatOperator.loadChatModel();
    if (!ret) {
        Logger::error("Llama Chat Model loading failed!");
    }

    return ret;

}

void InteractiveChatService::service_function()
{
    load_model();

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
            Logger::warn("{} subcribed_data_receive unknown message type!", get_service_name());
            break;
    }

}

void InteractiveChatService::llm_query(const std::string &query) {
    if (_queryRunning) {
        Logger::warn("llama Query is already running!");
    }else {
        Logger::info("Llama Query is started!");
        _queryRunning = true;
        send_query(query);
    }

}

