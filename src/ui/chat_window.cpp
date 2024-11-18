#include "chat_window.h"

ChatWindow::ChatWindow(finalcut::FWidget *parent): SubWindow(parent)
{
    setText("Chat Window");

    _promt.setGeometry(finalcut::FPoint{2,2}, finalcut::FSize{52, 1});
    _sendButton.setGeometry(finalcut::FPoint{2,4}, finalcut::FSize{7, 2});
    _sendButton.setText("query");
    _answerView.setGeometry(finalcut::FPoint{2,7}, finalcut::FSize{52, 26});

    add_clicked_callback(&_sendButton, this, &ChatWindow::send_promt);
    _interactiveChatService = InteractiveChatService::get_instance();

}

ChatWindow::~ChatWindow() noexcept
{

}

void ChatWindow::send_promt()
{
    std::string promt = _promt.getText().toString();

    promt = _interactiveChatService->query(promt);
    _answerView.clear();
    _answerView.append(promt);
    _answerView.redraw();

}
