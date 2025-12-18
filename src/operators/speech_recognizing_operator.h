/*
* Updated header file for optimized speech recognition
*/

#ifndef SPEECH_RECOGNIZING_OPERATOR_H
#define SPEECH_RECOGNIZING_OPERATOR_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <portaudio.h>
#include <queue>
#include <string>
#include <vector>
#include <vosk_api.h>

class SpeechRecognizingOperator {
private:
    static SpeechRecognizingOperator* _instance;
    
    // Vosk components
    VoskModel* _model;
    VoskRecognizer* _recognizer;

    // PortAudio components
    PaStream* _stream;
    
    // Thread synchronization
    std::mutex _bufferMutex;
    std::condition_variable _incomingBufferCv;
    std::string _wakeCommand;
    // Wake word and state management
    uint32_t _sampleRate;   //örnekleme frekansı), Specifies the number of recorded audio samples. 16000 is standard for speech recognition.
    uint32_t _framesPerBuffer; // Reduced for lower latency
    uint32_t _silenceThreshold;  // Silence detection threshold
    uint32_t _maxSilenceDurationSec; // Seconds of silence before stopping
    uint32_t _listenTimeoutMs; // getting message timeout
    std::vector<int16_t> _processBuffer;
    std::deque<std::vector<int16_t>> _bufferQueue;

    std::chrono::steady_clock::time_point _lastActivityTime;
    
    // Audio buffer for optimization
    std::vector<short> _audioBuffer;
    
    // Private constructor (Singleton)
    SpeechRecognizingOperator(uint32_t silenceThreshold,
                                 uint32_t sampleRate,
                                 uint32_t framesPerBuffer,
                                 uint32_t maxSilenceDurationSec,
                                 uint32_t listenTimeoutMs,
                                 const std::string &wakeCommand);
    
    // Helper methods
    bool is_human_speech_hybrid(const short* audio_data, size_t length);
    bool contains_wake_word(const std::string& text);

    // PortAudio callback
    static int paCallback(const void *input, void *output,
                         unsigned long frameCount,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData);

public:
    // Singleton access
    static SpeechRecognizingOperator* get_instance(uint32_t silenceThreshold = 500,
                                 uint32_t sampleRate = 16000,
                                 uint32_t framesPerBuffer = 2000,
                                 uint32_t maxSilenceDurationSec = 10,
                                 uint32_t listenTimeoutMs = 1000,
                                 const std::string &wakeCommand = "Kofi");
    
    ~SpeechRecognizingOperator();
    
    void load_model(const std::string &modelPath);
    bool open();
    bool start_listen();
    void stop_listen();
    void close();
    std::vector<int16_t> get_buffer();
    std::string get_text(const std::vector<int16_t> &audioData);


    static std::atomic<bool> _isSpeaking;

};

#endif // SPEECH_RECOGNIZING_OPERATOR_H