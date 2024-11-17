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
    struct RunConfig {
        std::filesystem::path modelPath;
        std::filesystem::path modelConfigPath;
        OutputType outputType = OUTPUT_DIRECTORY;
        std::optional<std::filesystem::path> outputPath = std::filesystem::path(".");
        std::optional<piper::SpeakerId> speakerId;
        std::optional<float> noiseScale;
        std::optional<float> lengthScale;
        std::optional<float> noiseW;
        std::optional<float> sentenceSilenceSeconds;
        std::optional<std::filesystem::path> eSpeakDataPath;
        std::optional<std::filesystem::path> tashkeelModelPath;
        bool jsonInput = false;
        std::optional<std::map<piper::Phoneme, float>> phonemeSilenceSeconds;
        bool useCuda = false;
    };

    static SpeechProcessController* get_instance();
    void loadModel(const std::string& modelPath, const std::string& modelConfigPath);
    void speakText(const std::string& text);
    void synthesizeText(const std::string& text);
    void playAudio();
    void playMusic(const std::string& mp3_file);

    ~SpeechProcessController();

private:
    RunConfig runConfig;
    piper::PiperConfig piperConfig;
    piper::Voice voice;
    std::vector<int16_t> audioBuffer;
    std::vector<int16_t> sharedAudioBuffer;
    std::mutex mutAudio;
    std::condition_variable cvAudio;
    bool audioReady = false;
    bool audioFinished = false;
    static SpeechProcessController* _instance;
    mpg123_handle* mh;  // MPG123 handle for MP3 playback
    int mpg123_err;

    
    SpeechProcessController();
    void audioCallbackFunc();
};

#endif // PIPERTTS_HPP

    // SpeechProcessController tts;
    
    // tts.loadModel("../ai.models/trSpeechModel/dfki.onnx",
    //               "../ai.models/trSpeechModel/dfki.onnx.json");

    // tts.speakText("Merhaba, bu bir test mesajıdır.");