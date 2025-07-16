#include "chat_window.h"

ChatWindow::ChatWindow(finalcut::FWidget *parent): SubWindow(parent)
{
    setText("Chat Window");

    _loadModelButton.setGeometry(finalcut::FPoint{64,2}, finalcut::FSize{7, 1});
    _loadModelButton.setText("load");

    _llamaChatModelPathLabel.setGeometry(finalcut::FPoint{2,2}, finalcut::FSize{7, 1});
    _llamaChatModelPath.setGeometry(finalcut::FPoint{10,2}, finalcut::FSize{52, 1});

    _llamaEmbeddingModelPathLabel.setGeometry(finalcut::FPoint{2,4}, finalcut::FSize{7, 1});
    _llamaEmbeddingModelPath.setGeometry(finalcut::FPoint{10,4}, finalcut::FSize{52, 1});

    _systemMessageLabel.setGeometry(finalcut::FPoint{2,6}, finalcut::FSize{7, 1});
    _systemMessage.setGeometry(finalcut::FPoint{10,6}, finalcut::FSize{52, 1});

    _temperatureLabel.setGeometry(finalcut::FPoint{2,8}, finalcut::FSize{7, 1});
    _temperature.setGeometry(finalcut::FPoint{10,8}, finalcut::FSize{7, 1});

    _maxTokenLabel.setGeometry(finalcut::FPoint{20,8}, finalcut::FSize{7, 1});
    _maxTokenSize.setGeometry(finalcut::FPoint{28,8}, finalcut::FSize{7, 1});

    _nthreadslabel.setGeometry(finalcut::FPoint{38,8}, finalcut::FSize{7, 1});
    _nThreads.setGeometry(finalcut::FPoint{46,8}, finalcut::FSize{7, 1});

    _topKLabel.setGeometry(finalcut::FPoint{2,10}, finalcut::FSize{7, 1});
    _topK.setGeometry(finalcut::FPoint{10,10}, finalcut::FSize{7, 1});

    _topPLabel.setGeometry(finalcut::FPoint{20,10}, finalcut::FSize{7, 1});
    _topP.setGeometry(finalcut::FPoint{28,10}, finalcut::FSize{7, 1});

    _poolingTypeLabel.setGeometry(finalcut::FPoint{38,10}, finalcut::FSize{7, 1});
    _poolingType.setGeometry(finalcut::FPoint{46,10}, finalcut::FSize{7, 1});

    _promtLabel.setGeometry(finalcut::FPoint{2,12}, finalcut::FSize{7, 1});
    _promt.setGeometry(finalcut::FPoint{10,12}, finalcut::FSize{52, 1});

    _sendButton.setGeometry(finalcut::FPoint{64,12}, finalcut::FSize{7, 1});
    _sendButton.setText("query");
    _answerView.setGeometry(finalcut::FPoint{2,14}, finalcut::FSize{71, 26});

    add_clicked_callback(&_sendButton, this, &ChatWindow::send_promt);
    add_clicked_callback(&_loadModelButton, this, &ChatWindow::load_model);

}

ChatWindow::~ChatWindow() noexcept
{

}

void ChatWindow::send_promt()
{
    _llmQueryFunctionCallBack(_promt.getText().toString());
}

void ChatWindow::load_model()
{

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

void ChatWindow::llm_response_callback(const std::string &response) {

    _answerView.append(response);
    _answerView.redraw();
}
