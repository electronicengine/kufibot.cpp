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

#include "web_socket_service.h"
#include <memory>
#include <variant>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

#include "remote_connection_service.h"

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
    _broadcastResponder = std::make_unique<BroadcastResponder>();

    _server.clear_access_channels(websocketpp::log::alevel::all);
    _server.set_open_handler(bind(&WebSocketService::on_open, this, std::placeholders::_1));
    _server.set_close_handler(bind(&WebSocketService::on_close, this, std::placeholders::_1));
    _server.set_message_handler(
            bind(&WebSocketService::on_message, this, std::placeholders::_1, std::placeholders::_2));
}
bool WebSocketService::initialize() {

    INFO("WebSocketService is initializing...");
    _broadcastResponder->start();

    RemoteConnectionService* _remoteConnectionService = RemoteConnectionService::get_instance();
    subscribe_to_service(_remoteConnectionService);
    return true;
}

void WebSocketService::service_function() {
    std::string address = "0.0.0.0";
    uint16_t port = 8080;
    INFO("WebSocketService is starting...");

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
            ERROR("Error resolving address: " + std::string(ec.message()));
            return;
        }

        // for (const auto& endpoint : endpoints) {
            _server.listen(boost::asio::ip::tcp::v4(), port); 
        // }

        _server.start_accept();
        INFO("WebSocket Server listening on " + address + ":" + std::to_string(port));

        _server.run(); // Blocking call to run the server
    } catch (websocketpp::exception const& e) {
        ERROR("WebSocket++ exception: " + std::string(e.what()));
    } catch (...) {
        ERROR("Other exception occurred");
    }

}


void WebSocketService::on_open(websocketpp::connection_hdl hdl) {
    WARNING("Connection opened!");

    std::unique_ptr<MessageData> data = std::make_unique<SensorData>();
    static_cast<WebSocketReceiveData*>(data.get())->msg = "on_open";
    static_cast<WebSocketReceiveData*>(data.get())->hdl = hdl;
    publish(MessageType::WebSocketReceive, data);
}

void WebSocketService::on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg) {
    std::unique_ptr<MessageData> data = std::make_unique<SensorData>();
    static_cast<WebSocketReceiveData*>(data.get())->msg = msg->get_payload();
    static_cast<WebSocketReceiveData*>(data.get())->hdl = hdl;
    publish(MessageType::WebSocketReceive, data);

}

void WebSocketService::subcribed_data_receive(MessageType type, const std::unique_ptr<MessageData> &data) {

    switch (type) {
        case MessageType::WebSocketTransfer: {
            if(data) {
                std::lock_guard<std::mutex> lock(_dataMutex);

                websocketpp::connection_hdl hdl = static_cast<WebSocketTransferData*>(data.get())->hdl;
                std::string mg = static_cast<WebSocketTransferData*>(data.get())->msg;
                uint8_t dataType =static_cast<WebSocketTransferData*>(data.get())->type;

                if (dataType == 1)
                    send_message(hdl, mg);
                else if (dataType == 2) {
                    std::vector<uchar> buffer(mg.begin(), mg.end());
                    send_data(hdl, buffer);
                }
            }
            break;
        }
        default:
            break;
    }
}



void WebSocketService::on_close(websocketpp::connection_hdl hdl) {
    WARNING("Connection closed!");

    std::unique_ptr<MessageData> data = std::make_unique<SensorData>();
    static_cast<WebSocketReceiveData*>(data.get())->msg = "on_close";
    static_cast<WebSocketReceiveData*>(data.get())->hdl = hdl;
    publish(MessageType::WebSocketReceive, data);
}

void WebSocketService::send_message(websocketpp::connection_hdl hdl, const std::string& message) {
    try {
        _server.send(hdl, message, websocketpp::frame::opcode::text);
    } catch (websocketpp::exception const& e) {
        INFO("Send failed: " + std::string(e.what()));
    }
}

void WebSocketService::send_data(websocketpp::connection_hdl hdl, const std::vector<uchar>& buffer) {
    try {
        _server.send(hdl, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary);
    } catch (websocketpp::exception const& e) {
        INFO("Send failed: " + std::string(e.what()));
    }
}

