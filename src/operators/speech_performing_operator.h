#ifndef SPEECH_PERFORMING_OPERATOR_H
#define SPEECH_PERFORMING_OPERATOR_H

#include <iostream>
#include <alsa/asoundlib.h>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>
#include <mpg123.h>
#include <alsa/asoundlib.h>

#include "piper.hpp"
#include "../public_data_messages.h"

#define TR_SPEECH_MODEL_PATH "/usr/local/ai.models/trSpeechModel/dfki.onnx"
#define ENG_SPEECH_MODEL_PATH "/usr/local/ai.models/engSpeechModel/en_GB-alan-low.onnx"

enum OutputType { OUTPUT_FILE, OUTPUT_DIRECTORY, OUTPUT_STDOUT, OUTPUT_RAW };

class SpeechPerformingOperator {
public:

    static SpeechPerformingOperator* get_instance();
    void loadModel(const std::string& modelPath = ENG_SPEECH_MODEL_PATH);
    void speakText(const std::string& text);
    void synthesizeText(const std::string& text);
    void playAudio();
    void playMusic(const std::string& mp3_file);

    ~SpeechPerformingOperator();

private:
    piper::PiperConfig _piperConfig;
    piper::Voice _voice;
    std::string _modelPath;
    std::vector<int16_t> _audioBuffer;
    std::vector<int16_t> _sharedAudioBuffer;
    std::optional<piper::SpeakerId> _speakerId;
    std::mutex _mutAudio;
    std::condition_variable _cvAudio;
    bool audioReady = false;
    bool audioFinished = false;
    static SpeechPerformingOperator* _instance;
    mpg123_handle* _mh;  // MPG123 handle for MP3 playback
    int _mpg123Err;
    bool keepAliveRunning;
    std::thread keepAliveThread;

    void stopKeepAlive();
    void keepAliveLoop();
    void startKeepAlive();
    SpeechPerformingOperator();
    void audioCallbackFunc();
};

#endif // SPEECH_PERFORMING_OPERATOR_H
