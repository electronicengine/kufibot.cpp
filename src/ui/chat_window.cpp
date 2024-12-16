#include "chat_window.h"

ChatWindow::ChatWindow(finalcut::FWidget *parent): SubWindow(parent)
{
    setText("Chat Window");

    _llamaServer.setGeometry(finalcut::FPoint{2,2}, finalcut::FSize{52, 1});
    _setServerButton.setGeometry(finalcut::FPoint{56,2}, finalcut::FSize{7, 1});
    _setServerButton.setText("set");
    _loadModelButton.setGeometry(finalcut::FPoint{64,2}, finalcut::FSize{7, 1});
    _loadModelButton.setText("load");

    _promt.setGeometry(finalcut::FPoint{2,4}, finalcut::FSize{52, 1});
    _sendButton.setGeometry(finalcut::FPoint{56,4}, finalcut::FSize{7, 1});
    _sendButton.setText("query");
    _answerView.setGeometry(finalcut::FPoint{2,7}, finalcut::FSize{52, 26});

    add_clicked_callback(&_sendButton, this, &ChatWindow::send_promt);
    add_clicked_callback(&_setServerButton, this, &ChatWindow::set_server);
    add_clicked_callback(&_loadModelButton, this, &ChatWindow::load_model);

    _interactiveChatService = InteractiveChatService::get_instance();

}

ChatWindow::~ChatWindow() noexcept
{

}

void ChatWindow::send_promt()
{
    std::string promt = _promt.getText().toString();

    std::function<void(const std::string&)> response_callback =
        [&](const std::string& response) {

            _answerView.append(response);
            _answerView.redraw();
        };

    promt = _interactiveChatService->query(promt, response_callback);
}

void ChatWindow::set_server()
{
    std::string server = _llamaServer.getText().toString();
    _interactiveChatService->set_llama_server(server);
}


void ChatWindow::load_model()
{
    std::thread loadThread([this]() {
        _interactiveChatService->load_model();
    });

    loadThread.detach();
}

void ChatWindow::onKeyPress (finalcut::FKeyEvent* ev)
{
    const FKey key = ev->key();
    _answerView.append("entered");
    _answerView.redraw();
    if ( isEnterKey(key) ){
        send_promt();
        ev->accept();
        return;
    }

    FDialog::onKeyPress(ev);
}
