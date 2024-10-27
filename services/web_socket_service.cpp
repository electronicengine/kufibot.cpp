#include "web_socket_service.h"
#include <memory>

WebSocketService* WebSocketService::_instance = nullptr;


WebSocketService *WebSocketService::get_instance()
{
    if (_instance == nullptr) {
        _instance = new WebSocketService();
    }
    return _instance;
}

WebSocketService::WebSocketService() {
    _server.init_asio();
    _server.clear_access_channels(websocketpp::log::alevel::all);
    _server.set_open_handler(bind(&WebSocketService::on_open, this, std::placeholders::_1));
    _server.set_close_handler(bind(&WebSocketService::on_close, this, std::placeholders::_1)); 
    _server.set_message_handler(bind(&WebSocketService::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

void WebSocketService::start(const std::string& address, uint16_t port) {

    if (!_running){
        _running = true;
        std::cout << "WebSocketService is starting..." << std::endl;
        _serverThread = std::thread(&WebSocketService::run, this, address, port); // Start server in a new thread
    }
}

void WebSocketService::run(const std::string &address, uint16_t port)
{
    try {
        // Create an endpoint to listen on a specific address and port
        websocketpp::lib::error_code ec;

        _server.set_reuse_addr(true); // Allow address reuse

        // Resolve the address
        boost::asio::ip::tcp::resolver resolver(_server.get_io_service());
        boost::asio::ip::tcp::resolver::results_type endpoints =  resolver.resolve(address, std::to_string(port));
        if (ec) {
            std::cerr << "Error resolving address: " << ec.message() << std::endl;
            return;
        }

        for (const auto& endpoint : endpoints) {
            _server.listen(endpoint.endpoint()); // Bind to the specific IP address and port
        }

        _server.start_accept();
        std::cout << "WebSocket Server listening on " << address << ":" << port << std::endl;

        _server.run(); // Blocking call to run the server
    } catch (websocketpp::exception const& e) {
        std::cerr << "WebSocket++ exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Other exception occurred" << std::endl;
    }

    _running = false; // Set flag to false when server stops
}

void WebSocketService::stop() {

    if (_running){

        _running = false;
        std::cout << "WebSocketService is stopping..." << std::endl;
        _server.stop_listening();  // Stop the server from accepting new connections
        _server.stop();            // Stop the server event loop

        if (_serverThread.joinable()) {
            _serverThread.join();  // Wait for the thread to finish
        }
        std::cout << "WebSocketService is stopped." << std::endl;
    }

}


void WebSocketService::on_open(websocketpp::connection_hdl hdl) {
    std::cout << "Connection opened!" << std::endl;
    _hdl = hdl;  
   std::string msg = "on_open";
    update_web_socket_message(hdl, msg);
}

void WebSocketService::on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg) {
    update_web_socket_message(hdl, msg->get_payload());
}

void WebSocketService::on_close(websocketpp::connection_hdl hdl) {
    std::cout << "Connection closed!" << std::endl;
    std::string msg =  "on_close";
    update_web_socket_message(hdl, msg);
}

void WebSocketService::send_message(websocketpp::connection_hdl hdl, const std::string& message) {
    try {
        _server.send(hdl, message, websocketpp::frame::opcode::text);
    } catch (websocketpp::exception const& e) {
        std::cerr << "Send failed: " << e.what() << std::endl;
    }
}

void WebSocketService::send_data(websocketpp::connection_hdl hdl, const std::vector<uchar>& buffer) {
    try {
        _server.send(hdl, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary);
    } catch (websocketpp::exception const& e) {
        std::cerr << "Send failed: " << e.what() << std::endl;
    }
}

