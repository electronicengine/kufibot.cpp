// VideoStream.h
#ifndef TUI_SERVICE_H
#define TUI_SERVICE_H

#include "service.h"

class TuiService : public Service{

public:
    virtual ~TuiService();

    static TuiService *get_instance();


private:
    static TuiService* _instance;

    TuiService();

    void service_function();

    //subscribed sensor_data, llm_response
    void sensor_data(Json values);
    void llm_response(const std::string& response);

    //subscribed sensor_data, llm_response
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);

};

#endif // TUI_SERVICE_H