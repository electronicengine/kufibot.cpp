#ifndef SPEECH_RECOGNIZING_OPERATOR_H
#define SPEECH_RECOGNIZING_OPERATOR_H

#include <iostream>
#include <portaudio.h>
#include "vosk_api.h"
#include <mutex>
#include <condition_variable>
#include <string>


#define SAMPLE_RATE 16000
#define FRAMES_PER_BUFFER 8000
#define TR_RECOGNIZE_MODEL_PATH "/usr/local/ai.models/trRecognizeModel"
class SpeechRecognizingOperator {
public:
    ~SpeechRecognizingOperator();

    void load_model(const std::string &modelPath = TR_RECOGNIZE_MODEL_PATH);
    bool open();              // Initialize resources
    bool start_listen();       // Start the audio stream
    void stop_listen();        // Stop the audio stream
    std::string get_message(); // Wait and return the next final message
    void close();              // Clean up resources
    static SpeechRecognizingOperator* get_instance();

private:
    VoskModel *_model;
    VoskRecognizer *_recognizer;
    PaStream *_stream;

    std::string _lastMessage;
    std::mutex _messageMutex;
    std::condition_variable _messageCv;
    bool _messageReady = false;  // Flag to track if a new message is available
    static SpeechRecognizingOperator* _instance;

    SpeechRecognizingOperator();
    static int paCallback(const void *input, void *output,
                        unsigned long frameCount,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData);


    void store_message(const std::string &message); // Store new message and notify

};
#endif //SPEECH_RECOGNIZING_OPERATOR_H
