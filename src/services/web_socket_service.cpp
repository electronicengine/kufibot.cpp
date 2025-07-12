#include "web_socket_service.h"
#include <memory>
#include "../logger.h"

WebSocketService* WebSocketService::_instance = nullptr;


WebSocketService *WebSocketService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new WebSocketService();
    }
    return _instance;
}

WebSocketService::WebSocketService() : Service("WebSocketService") {
    _server.init_asio();
    _server.clear_access_channels(websocketpp::log::alevel::all);
    _server.set_open_handler(bind(&WebSocketService::on_open, this, std::placeholders::_1));
    _server.set_close_handler(bind(&WebSocketService::on_close, this, std::placeholders::_1)); 
    _server.set_message_handler(bind(&WebSocketService::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

void WebSocketService::start(const std::string& address, uint16_t port) {

    if (!_running){
        _running = true;
       Logger::info("WebSocketService is starting...");
        _serviceThread = std::thread(&WebSocketService::run, this, address, port); // Start server in a new thread
    }
}

void WebSocketService::run(const std::string &address, uint16_t port)
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

    _running = false;
}

void WebSocketService::stop() {

    if (_running){

        _running = false;
        Logger::info("WebSocketService is stopping...");
        _server.stop_listening();  
        _server.stop();            

        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }
        Logger::info("WebSocketService is stopped.");
    }

}


void WebSocketService::on_open(websocketpp::connection_hdl hdl) {
    Logger::info("Connection opened!");
    _hdl = hdl;  
   std::string msg = "on_open";
    update_web_socket_message(hdl, msg);
}

void WebSocketService::on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg) {
    update_web_socket_message(hdl, msg->get_payload());
}

void WebSocketService::on_close(websocketpp::connection_hdl hdl) {
    Logger::info("Connection closed!");
    std::string msg =  "on_close";
    update_web_socket_message(hdl, msg);
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

