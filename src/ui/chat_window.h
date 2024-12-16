
#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include "final/final.h"
#include "sub_window.h"
#include <deque>

#undef K
#undef null

#include "../services/interactive_chat_service.h"

using namespace finalcut;

class ChatWindow : public SubWindow
{
  public:
    explicit ChatWindow (finalcut::FWidget* = nullptr);

    ChatWindow (const ChatWindow&) = delete;

    ChatWindow (ChatWindow&&) noexcept = delete;

    ~ChatWindow() noexcept override;

    auto operator = (const ChatWindow&) -> ChatWindow& = delete;

    auto operator = (ChatWindow&&) noexcept -> ChatWindow& = delete;

    FLineEdit _llamaServer{"http://localhost:11434-kufi", this};

    FLineEdit _promt{"", this};
    FTextView _answerView{this};

    FButton _sendButton{this};
    FButton _setServerButton{this};
    FButton _loadModelButton{this};

    InteractiveChatService *_interactiveChatService;

    void send_promt();
    void set_server();
    void load_model();
    void onKeyPress (finalcut::FKeyEvent* ev);

};

#endif // CHATWINDOW_H