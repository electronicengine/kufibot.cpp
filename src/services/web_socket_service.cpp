#include "web_socket_service.h"
#include <memory>
#include "../ui/main_window.h"

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
        MainWindow::log("WebSocketService is starting...", LogLevel::LOG_INFO);
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
            MainWindow::log("Error resolving address: " + std::string(ec.message()), LogLevel::LOG_ERROR);
            return;
        }

        // for (const auto& endpoint : endpoints) {
            _server.listen(boost::asio::ip::tcp::v4(), port); 
        // }

        _server.start_accept();
        MainWindow::log("WebSocket Server listening on " + address + ":" + std::to_string(port), LogLevel::LOG_INFO);

        _server.run(); // Blocking call to run the server
    } catch (websocketpp::exception const& e) {
        MainWindow::log("WebSocket++ exception: " + std::string(e.what()), LogLevel::LOG_ERROR);
    } catch (...) {
        MainWindow::log("Other exception occurred", LogLevel::LOG_ERROR);
    }

    _running = false;
}

void WebSocketService::stop() {

    if (_running){

        _running = false;
        MainWindow::log("WebSocketService is stopping...", LogLevel::LOG_INFO);
        _server.stop_listening();  
        _server.stop();            

        if (_serviceThread.joinable()) {
            _serviceThread.join(); 
        }
        MainWindow::log("WebSocketService is stopped.", LogLevel::LOG_INFO);
    }

}


void WebSocketService::on_open(websocketpp::connection_hdl hdl) {
    MainWindow::log("Connection opened!", LogLevel::LOG_INFO);
    _hdl = hdl;  
   std::string msg = "on_open";
    update_web_socket_message(hdl, msg);
}

void WebSocketService::on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg) {
    update_web_socket_message(hdl, msg->get_payload());
}

void WebSocketService::on_close(websocketpp::connection_hdl hdl) {
    MainWindow::log("Connection closed!", LogLevel::LOG_INFO);
    std::string msg =  "on_close";
    update_web_socket_message(hdl, msg);
}

void WebSocketService::send_message(websocketpp::connection_hdl hdl, const std::string& message) {
    try {
        _server.send(hdl, message, websocketpp::frame::opcode::text);
    } catch (websocketpp::exception const& e) {
        MainWindow::log("Send failed: " + std::string(e.what()), LogLevel::LOG_ERROR);
    }
}

void WebSocketService::send_data(websocketpp::connection_hdl hdl, const std::vector<uchar>& buffer) {
    try {
        _server.send(hdl, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary);
    } catch (websocketpp::exception const& e) {
        MainWindow::log("Send failed: " + std::string(e.what()), LogLevel::LOG_ERROR);
    }
}

