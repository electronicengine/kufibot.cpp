/*
* Updated header file for optimized speech recognition
*/

#ifndef SPEECH_RECOGNIZING_OPERATOR_H
#define SPEECH_RECOGNIZING_OPERATOR_H

#include <string>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <portaudio.h>
#include <vosk_api.h>

#define TR_RECOGNIZE_MODEL_PATH "/usr/local/ai.models/engRecognizeModel"


class SpeechRecognizingOperator {
private:
    static SpeechRecognizingOperator* _instance;
    
    // Vosk components
    VoskModel* _model;
    VoskRecognizer* _recognizer;
    VoskRecognizer* _wakeWordRecognizer;  // Separate recognizer for wake word
    
    // PortAudio components
    PaStream* _stream;
    
    // Thread synchronization
    std::mutex _messageMutex;
    std::condition_variable _messageCv;
    std::string _lastMessage;
    bool _messageReady;
    
    // Wake word and state management
    bool _isWakeWordMode;
    bool _isListening;
    std::chrono::steady_clock::time_point _lastActivityTime;
    
    // Audio buffer for optimization
    std::vector<short> _audioBuffer;
    
    // Private constructor (Singleton)
    SpeechRecognizingOperator();
    
    // Helper methods
    bool detect_voice_activity(const short* audio_data, size_t length);
    bool contains_wake_word(const std::string& text);
    void store_message(const std::string &message);
    
    // PortAudio callback
    static int paCallback(const void *input, void *output,
                         unsigned long frameCount,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData);

public:
    // Singleton access
    static SpeechRecognizingOperator* get_instance();
    
    // Destructor
    ~SpeechRecognizingOperator();
    
    // Core functionality
    void load_model(const std::string &modelPath);
    bool open();
    bool start_listen();
    void stop_listen();
    void close();
    void setListeningMode(bool isListening);
    
    // Message retrieval with optional timeout
    std::string get_message(int timeout_ms = 0);
    std::string get_partial_result();
    
    // Wake word control
    void reset_to_wake_word_mode();
    
    // State queries
    bool is_listening() const { return _isListening; }
    bool is_wake_word_mode() const { return _isWakeWordMode; }
};

#endif // SPEECH_RECOGNIZING_OPERATOR_H