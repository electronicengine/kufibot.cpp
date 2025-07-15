#include "web_socket_service.h"
#include <memory>
#include <variant>

#include "remote_connection_service.h"
#include "../logger.h"

WebSocketService* WebSocketService::_instance = nullptr;


WebSocketService *WebSocketService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new WebSocketService();
    }
    return _instance;
}

WebSocketService::~WebSocketService() {

}

WebSocketService::WebSocketService() : Service("WebSocketService") {
    _server.init_asio();
    _server.clear_access_channels(websocketpp::log::alevel::all);
    _server.set_open_handler(bind(&WebSocketService::on_open, this, std::placeholders::_1));
    _server.set_close_handler(bind(&WebSocketService::on_close, this, std::placeholders::_1)); 
    _server.set_message_handler(bind(&WebSocketService::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

void WebSocketService::service_function() {
    std::string address = "192.168.1.44";
    uint16_t port = 8080;

    Logger::info("WebSocketService is starting...");
    RemoteConnectionService* _remoteConnectionService = RemoteConnectionService::get_instance();
    subscribe_to_service(_remoteConnectionService);

    run_web_server(address, port);
}


void WebSocketService::run_web_server(const std::string &address, uint16_t port)
{
    try {
        websocketpp::lib::error_code ec;

        _server.set_reuse_addr(true); 


        boost::asio::ip::tcp::resolver resolver(_server.get_io_service());
        boost::asio::ip::tcp::resolver::results_type endpoints =  resolver.resolve(address, std::to_string(port));
        if (ec) {
            Logger::error("Error resolving address: " + std::string(ec.message()));
            return;
        }

        // for (const auto& endpoint : endpoints) {
            _server.listen(boost::asio::ip::tcp::v4(), port); 
        // }

        _server.start_accept();
        Logger::info("WebSocket Server listening on " + address + ":" + std::to_string(port));

        _server.run(); // Blocking call to run the server
    } catch (websocketpp::exception const& e) {
        Logger::error("WebSocket++ exception: " + std::string(e.what()));
    } catch (...) {
        Logger::error("Other exception occurred");
    }

}


void WebSocketService::on_open(websocketpp::connection_hdl hdl) {
    Logger::info("Connection opened!");
    WebSocketReceiveData *data = new WebSocketReceiveData();
    data->msg ="on_open";
    data->hdl = hdl;
    publish(MessageType::WebSocketReceive, data);
}

void WebSocketService::on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg) {
    WebSocketReceiveData *data = new WebSocketReceiveData();
    data->msg = msg->get_payload();
    data->hdl = hdl;
    publish(MessageType::WebSocketReceive, data);
}

void WebSocketService::subcribed_data_receive(MessageType type, MessageData *data) {
    std::lock_guard<std::mutex> lock(_dataMutex);

    switch (type) {
        case MessageType::WebSocketTransfer: {
            if(data) {

                websocketpp::connection_hdl &hdl = static_cast<WebSocketTransferData*>(data)->hdl;
                std::string &mg = static_cast<WebSocketTransferData*>(data)->msg;
                uint8_t &dataType =static_cast<WebSocketTransferData*>(data)->type;

                if (dataType == 1)
                    send_message(hdl, mg);
                else if (dataType == 2) {
                    std::vector<uchar> buffer(mg.begin(), mg.end());
                    send_data(hdl, buffer);
                }
            }
            break;
        }
    }
}



void WebSocketService::on_close(websocketpp::connection_hdl hdl) {
    Logger::info("Connection closed!");

    WebSocketReceiveData *data = new WebSocketReceiveData();
    data->msg =  "on_close";
    data->hdl = hdl;
    publish(MessageType::WebSocketReceive, data);

}

void WebSocketService::send_message(websocketpp::connection_hdl hdl, const std::string& message) {
    try {
        _server.send(hdl, message, websocketpp::frame::opcode::text);
    } catch (websocketpp::exception const& e) {
        Logger::info("Send failed: " + std::string(e.what()));
    }
}

void WebSocketService::send_data(websocketpp::connection_hdl hdl, const std::vector<uchar>& buffer) {
    try {
        _server.send(hdl, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary);
    } catch (websocketpp::exception const& e) {
        Logger::info("Send failed: " + std::string(e.what()));
    }
}

