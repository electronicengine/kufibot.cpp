#ifndef SPEECH_PROCESS_CONTROLLER_HPP
#define SPEECH_PROCESS_CONTROLLER_HPP

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


enum OutputType { OUTPUT_FILE, OUTPUT_DIRECTORY, OUTPUT_STDOUT, OUTPUT_RAW };

class SpeechProcessController {
public:

    static SpeechProcessController* get_instance();
    void loadModel(const std::string& modelPath = "/usr/ai.models/trSpeechModel/dfki.onnx");
    void speakText(const std::string& text);
    void synthesizeText(const std::string& text);
    void playAudio();
    void playMusic(const std::string& mp3_file);

    ~SpeechProcessController();

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
    static SpeechProcessController* _instance;
    mpg123_handle* _mh;  // MPG123 handle for MP3 playback
    int _mpg123Err;

    
    SpeechProcessController();
    void audioCallbackFunc();
};

#endif // PIPERTTS_HPP
