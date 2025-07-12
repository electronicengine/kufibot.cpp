
#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include "final/final.h"
#include "sub_window.h"
#include <deque>

#undef K
#undef null

#include "../services/interactive_chat_service.h"

using namespace finalcut;



class ChatWindow : public SubWindow {
public:
    explicit ChatWindow (finalcut::FWidget* = nullptr);

    ChatWindow (const ChatWindow&) = delete;

    ChatWindow (ChatWindow&&) noexcept = delete;

    ~ChatWindow() noexcept override;

    auto operator = (const ChatWindow&) -> ChatWindow& = delete;

    auto operator = (ChatWindow&&) noexcept -> ChatWindow& = delete;

    FLabel _llamaChatModelPathLabel{"Chat", this};
    FLineEdit _llamaChatModelPath{"/usr/ai.models/trLlamaModel/dolphin3.gguf", this};

    FLabel _llamaEmbeddingModelPathLabel{"Embed", this};
    FLineEdit _llamaEmbeddingModelPath{"/usr/ai.models/trLlamaModel/mxbiaV1.gguf", this};

    FLabel _systemMessageLabel{"Sys.", this};
    FLineEdit _systemMessage{"Sen yardımcı bir yapay zeka asistanısın.", this};

    FLabel _promtLabel{"Prompt", this};
    FLineEdit _promt{"", this};

    FLabel _temperatureLabel{"Temp.", this};
    FLineEdit _temperature{"0.2", this};

    FLabel _maxTokenLabel{"Token.", this};
    FLineEdit _maxTokenSize{"2048", this};

    FLabel _topKLabel{"topK", this};
    FLineEdit _topK{"50", this};

    FLabel _topPLabel{"topP", this};
    FLineEdit _topP{"0.9", this};

    FLabel _nthreadslabel{"thread.", this};
    FLineEdit _nThreads{"4", this};

    FLabel _poolingTypeLabel{"pooling.", this};
    FLineEdit _poolingType{"1", this};

    FTextView _answerView{this};
    FButton _sendButton{this};
    FButton _loadModelButton{this};

    InteractiveChatService *_interactiveChatService;

    void send_promt();
    void load_model();
    void onKeyPress (finalcut::FKeyEvent* ev);

};

#endif // CHATWINDOW_H