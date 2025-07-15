#include "speech_performing_operator.h"
#include "../logger.h"


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
    _stream = nullptr;
    _messageReady = false;
}

// Destructor
SpeechRecognizingOperator::~SpeechRecognizingOperator() {
    close();
}

// Store message and notify waiting threads
void SpeechRecognizingOperator::store_message(const std::string &message) {
    std::lock_guard<std::mutex> lock(_messageMutex);
    _lastMessage = message;
    _messageReady = true;
    _messageCv.notify_one(); // Notify waiting thread
}

// PortAudio Callback
int SpeechRecognizingOperator::paCallback(const void *input, void *output,
                                            unsigned long frameCount,
                                            const PaStreamCallbackTimeInfo *timeInfo,
                                            PaStreamCallbackFlags statusFlags,
                                            void *userData) {
    auto *controller = static_cast<SpeechRecognizingOperator *>(userData);

    if (vosk_recognizer_accept_waveform(controller->_recognizer, (const char *)input, frameCount * sizeof(short))) {
        std::string result = vosk_recognizer_result(controller->_recognizer);
        controller->store_message(result); // Store the final result
    } 
    return paContinue;
}

void SpeechRecognizingOperator::load_model(const std::string &modelPath)
{
    _model = vosk_model_new(modelPath.c_str());
    _recognizer = vosk_recognizer_new(_model, SAMPLE_RATE);
}

// Initialize PortAudio and prepare resources
bool SpeechRecognizingOperator::open() {
    if (Pa_Initialize() != paNoError) {
        Logger::error("Failed to initialize PortAudio.");
        return false;
    }

    PaError err = Pa_OpenDefaultStream(&_stream,
                                       1,          // Input channels
                                       0,          // Output channels
                                       paInt16,    // Sample format
                                       SAMPLE_RATE,
                                       FRAMES_PER_BUFFER,
                                       paCallback, // Callback function
                                       this        // User data (self)
    );

    if (err != paNoError) {
        Logger::error("Failed to open audio stream: {}", std::string( Pa_GetErrorText(err)));
        return false;
    }

    return true;
}

// Start listening to the microphone
bool SpeechRecognizingOperator::start_listen() {
    if (Pa_StartStream(_stream) != paNoError) {
        Logger::error("Failed to start audio stream.");
        return false;
    }
    Logger::error("Listening... Press Ctrl+C to stop.");

    return true;
}

// Stop listening
void SpeechRecognizingOperator::stop_listen() {
    Pa_StopStream(_stream);
}

// Wait and return the next final message
std::string SpeechRecognizingOperator::get_message() {
    std::unique_lock<std::mutex> lock(_messageMutex);
    _messageCv.wait(lock, [this]() { return _messageReady; }); // Block until message is ready
    _messageReady = false;  // Reset the flag
    return _lastMessage;
}

// Clean up and release resources
void SpeechRecognizingOperator::close() {
    if (_stream) {
        Pa_CloseStream(_stream);
    }
    Pa_Terminate();
    vosk_recognizer_free(_recognizer);
    vosk_model_free(_model);
}