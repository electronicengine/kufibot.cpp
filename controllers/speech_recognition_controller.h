#ifndef SPEECH_RECOGNITON_CONTROLLER_H
#define SPEECH_RECOGNITON_CONTROLLER_H

#include <iostream>
#include <portaudio.h>
#include "vosk_api.h"
#include <mutex>
#include <condition_variable>
#include <string>


#define SAMPLE_RATE 16000
#define FRAMES_PER_BUFFER 8000

class SpeechRecognitionController {
public:
    ~SpeechRecognitionController();

    void load_mode(const std::string &modelPath);
    bool open();              // Initialize resources
    bool start_listen();       // Start the audio stream
    void stop_listen();        // Stop the audio stream
    std::string get_message(); // Wait and return the next final message
    void close();              // Clean up resources
    static SpeechRecognitionController* get_instance();

private:
    VoskModel *_model;
    VoskRecognizer *_recognizer;
    PaStream *_stream;

    std::string _lastMessage;
    std::mutex _messageMutex;
    std::condition_variable _messageCv;
    bool _messageReady = false;  // Flag to track if a new message is available
    static SpeechRecognitionController* _instance;

    SpeechRecognitionController();
    static int paCallback(const void *input, void *output,
                        unsigned long frameCount,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData);


    void store_message(const std::string &message); // Store new message and notify

};
#endif

    // SpeechRecognitionController recognizer("../ai.models/trRecognizeModel");

    // if (!recognizer.open()) {
    //     return 1;
    // }

    // if (!recognizer.start_listen()) {
    //     recognizer.close();
    //     return 1;
    // }

    // while (true) {
    //     std::string message = recognizer.get_message();  // Wait for next message
    //     std::cout << "Recognized: " << message << std::endl;
    // }

    // recognizer.stop_listen();
    // recognizer.close();