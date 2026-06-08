#ifndef VOICE_AGENT_SERVICE_H
#define VOICE_AGENT_SERVICE_H

#include "service.h"
#include "../operators/UdpServerSocket.h"

class VoiceAgentService : public Service {
public:
    static VoiceAgentService* get_instance();
    virtual ~VoiceAgentService();

    virtual bool initialize() override;
    virtual void service_function() override;

private:
    VoiceAgentService();
    static VoiceAgentService* _instance;

    UdpServerSocket _udpServer;
    int _listenPort = 5005;
    std::shared_ptr<SensorData> _sensorData;

    void process_received_data(const std::string& data);
    void subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData>& data);

};

#endif // VOICE_AGENT_SERVICE_H
