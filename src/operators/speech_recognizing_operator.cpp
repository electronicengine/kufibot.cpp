/*
* Optimized Speech Recognition with Wake Word Detection
*/

#include "speech_recognizing_operator.h"
#include "../logger.h"
#include <algorithm>
#include <chrono>

// Optimized constants
#define SAMPLE_RATE 16000
#define FRAMES_PER_BUFFER 4000  // Reduced for lower latency
#define TR_RECOGNIZE_MODEL_PATH "/usr/local/ai.models/engRecognizeModel"
#define WAKE_WORD "coffee"  // Your wake word
#define SILENCE_THRESHOLD 500  // Silence detection threshold
#define MAX_SILENCE_DURATION 2.0  // Seconds of silence before stopping

SpeechRecognizingOperator* SpeechRecognizingOperator::_instance = nullptr;

SpeechRecognizingOperator* SpeechRecognizingOperator::get_instance() {
    if (_instance == nullptr) {
        _instance = new SpeechRecognizingOperator();
    }
    return _instance;
}

// Constructor
SpeechRecognizingOperator::SpeechRecognizingOperator(){
    _recognizer = nullptr;
    _wakeWordRecognizer = nullptr;
    _stream = nullptr;
    _messageReady = false;
    _isWakeWordMode = true;
    _isListening = false;
    _lastActivityTime = std::chrono::steady_clock::now();
    _audioBuffer.reserve(FRAMES_PER_BUFFER * 2);  // Pre-allocate buffer
}

// Destructor
SpeechRecognizingOperator::~SpeechRecognizingOperator() {
    close();
}

// Voice Activity Detection
bool SpeechRecognizingOperator::detect_voice_activity(const short* audio_data, size_t length) {
    // Simple energy-based VAD
    long long energy = 0;
    for (size_t i = 0; i < length; ++i) {
        energy += abs(audio_data[i]);
    }

    double average_energy = static_cast<double>(energy) / length;
    return average_energy > SILENCE_THRESHOLD;
}

// Check for wake word in recognition result
bool SpeechRecognizingOperator::contains_wake_word(const std::string& text) {
    std::string lower_text = text;
    std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
    return lower_text.find(WAKE_WORD) != std::string::npos;
}

// Store message and notify waiting threads
void SpeechRecognizingOperator::store_message(const std::string &message) {
    std::lock_guard<std::mutex> lock(_messageMutex);
    _lastMessage = message;
    _messageReady = true;
    _messageCv.notify_one();
}

// Optimized PortAudio Callback with Wake Word Detection
int SpeechRecognizingOperator::paCallback(const void *input, void *output,
                                            unsigned long frameCount,
                                            const PaStreamCallbackTimeInfo *timeInfo,
                                            PaStreamCallbackFlags statusFlags,
                                            void *userData) {
    auto *controller = static_cast<SpeechRecognizingOperator *>(userData);
    const short* audio_data = static_cast<const short*>(input);

    // Voice Activity Detection
    bool has_voice = controller->detect_voice_activity(audio_data, frameCount);

    if (has_voice) {
        controller->_lastActivityTime = std::chrono::steady_clock::now();
    }

    // Wake word mode - always listening for wake word
    if (controller->_isWakeWordMode) {
        if (vosk_recognizer_accept_waveform(controller->_wakeWordRecognizer,
                                          (const char *)input,
                                          frameCount * sizeof(short))) {
            std::string result = vosk_recognizer_result(controller->_wakeWordRecognizer);

            if (controller->contains_wake_word(result)) {
                INFO("Wake word detected! Starting full recognition...");
                controller->_isWakeWordMode = false;
                controller->_isListening = true;
                controller->_lastActivityTime = std::chrono::steady_clock::now();

                // Reset main recognizer for fresh start
                vosk_recognizer_reset(controller->_recognizer);
            }
        } else {
            // Process partial results for faster wake word detection
            std::string partial = vosk_recognizer_partial_result(controller->_wakeWordRecognizer);
            if (controller->contains_wake_word(partial)) {
                // Don't wait for final result if wake word is in partial
                INFO("Wake word detected in partial result! Starting recognition...");
                controller->_isWakeWordMode = false;
                controller->_isListening = true;
                controller->_lastActivityTime = std::chrono::steady_clock::now();
                vosk_recognizer_reset(controller->_recognizer);
            }
        }
    }
    // Full recognition mode - only when actively listening
    else if (controller->_isListening) {
        // Check for silence timeout
        auto now = std::chrono::steady_clock::now();
        auto silence_duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - controller->_lastActivityTime).count();

        if (silence_duration > MAX_SILENCE_DURATION) {
            INFO("Silence detected. Switching back to wake word mode.");
            controller->_isWakeWordMode = true;
            controller->_isListening = false;
            vosk_recognizer_reset(controller->_wakeWordRecognizer);
            return paContinue;
        }

        // Only process if there's voice activity (saves CPU)
        if (has_voice || silence_duration < 1.0) {  // Allow short pauses
            if (vosk_recognizer_accept_waveform(controller->_recognizer,
                                              (const char *)input,
                                              frameCount * sizeof(short))) {
                std::string result = vosk_recognizer_result(controller->_recognizer);
                controller->store_message(result);
            }
        }
    }

    return paContinue;
}

void SpeechRecognizingOperator::load_model(const std::string &modelPath) {
    // Load main model for full recognition
    _model = vosk_model_new(modelPath.c_str());
    _recognizer = vosk_recognizer_new(_model, SAMPLE_RATE);

    // Create lightweight recognizer for wake word detection
    _wakeWordRecognizer = vosk_recognizer_new(_model, SAMPLE_RATE);

    // Configure recognizers for optimal performance
    vosk_recognizer_set_max_alternatives(_recognizer, 1);  // Only best result
    vosk_recognizer_set_max_alternatives(_wakeWordRecognizer, 1);
}

// Initialize PortAudio with optimized settings
bool SpeechRecognizingOperator::open() {
    if (Pa_Initialize() != paNoError) {
        ERROR("Failed to initialize PortAudio.");
        return false;
    }

    // Get default input device info for optimization
    PaDeviceIndex defaultDevice = Pa_GetDefaultInputDevice();
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(defaultDevice);

    PaStreamParameters inputParameters;
    inputParameters.device = defaultDevice;
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paInt16;
    inputParameters.suggestedLatency = deviceInfo->defaultLowInputLatency;  // Low latency
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(&_stream,
                               &inputParameters,
                               nullptr,        // No output
                               SAMPLE_RATE,
                               FRAMES_PER_BUFFER,
                               paClipOff,      // No clipping
                               paCallback,
                               this);

    if (err != paNoError) {
        ERROR("Failed to open audio stream: {}", std::string(Pa_GetErrorText(err)));
        return false;
    }

    return true;
}

// Start listening with wake word detection
bool SpeechRecognizingOperator::start_listen() {
    if (Pa_StartStream(_stream) != paNoError) {
        ERROR("Failed to start audio stream.");
        return false;
    }

    _isWakeWordMode = true;
    _isListening = false;
    INFO("Wake word detection active. Say '{}' to start recognition...", WAKE_WORD);

    return true;
}

// Stop listening
void SpeechRecognizingOperator::stop_listen() {
    Pa_StopStream(_stream);
    _isListening = false;
    _isWakeWordMode = true;
}

// Get message with timeout for better responsiveness
std::string SpeechRecognizingOperator::get_message(int timeout_ms) {
    std::unique_lock<std::mutex> lock(_messageMutex);

    if (timeout_ms > 0) {
        auto timeout = std::chrono::milliseconds(timeout_ms);
        if (_messageCv.wait_for(lock, timeout, [this]() { return _messageReady; })) {
            _messageReady = false;
            return _lastMessage;
        }
        return "";  // Timeout
    } else {
        _messageCv.wait(lock, [this]() { return _messageReady; });
        _messageReady = false;
        return _lastMessage;
    }
}

// Get partial results for real-time feedback
std::string SpeechRecognizingOperator::get_partial_result() {
    if (_isListening && _recognizer) {
        return vosk_recognizer_partial_result(_recognizer);
    }
    return "";
}

// Force wake word mode (useful for manual control)
void SpeechRecognizingOperator::reset_to_wake_word_mode() {
    _isWakeWordMode = true;
    _isListening = false;
    if (_wakeWordRecognizer) {
        vosk_recognizer_reset(_wakeWordRecognizer);
    }
    INFO("Reset to wake word detection mode.");
}

// Clean up and release resources
void SpeechRecognizingOperator::close() {
    if (_stream) {
        Pa_CloseStream(_stream);
    }
    Pa_Terminate();

    if (_recognizer) {
        vosk_recognizer_free(_recognizer);
    }
    if (_wakeWordRecognizer) {
        vosk_recognizer_free(_wakeWordRecognizer);
    }
    if (_model) {
        vosk_model_free(_model);
    }
}