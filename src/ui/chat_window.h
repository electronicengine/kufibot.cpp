
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

    FLineEdit _promt{finalcut::UniChar::BlackUpPointingTriangle, this};
    FButton _sendButton{this};
    FTextView _answerView{this};

    InteractiveChatService *_interactiveChatService;

    void send_promt();

};

#endif // CHATWINDOW_H