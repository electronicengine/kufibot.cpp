/*
* Optimized Speech Recognition with Wake Word Detection
*/

#include "speech_recognizing_operator.h"
#include "../logger.h"
#include <algorithm>
#include <chrono>

#include "speech_performing_operator.h"

SpeechRecognizingOperator* SpeechRecognizingOperator::_instance = nullptr;
std::atomic<bool> SpeechRecognizingOperator::_isSpeaking = false;

SpeechRecognizingOperator* SpeechRecognizingOperator::get_instance(uint32_t silenceThreshold,
                                 uint32_t sampleRate,
                                 uint32_t framesPerBuffer,
                                 uint32_t maxSilenceDurationSec,
                                 uint32_t listenTimeoutMs,
                                 const std::string &wakeCommand) {
    if (_instance == nullptr) {
        _instance = new SpeechRecognizingOperator(silenceThreshold,sampleRate,framesPerBuffer,maxSilenceDurationSec,listenTimeoutMs, wakeCommand);
    }
    return _instance;
}

// Constructor
SpeechRecognizingOperator::SpeechRecognizingOperator(uint32_t silenceThreshold,
                                                     uint32_t sampleRate,
                                                     uint32_t framesPerBuffer,
                                                     uint32_t maxSilenceDurationSec,
                                                     uint32_t listenTimeoutMs,
                                                     const std::string &wakeCommand) {
    _silenceThreshold = silenceThreshold; ;  // Silence detection threshold
    _sampleRate = sampleRate;   //örnekleme frekansı), Specifies the number of recorded audio samples. 16000 is standard for speech recognition.
    _framesPerBuffer = framesPerBuffer; // Reduced for lower latency
    _maxSilenceDurationSec = maxSilenceDurationSec; // Seconds of silence before stopping
    _listenTimeoutMs = listenTimeoutMs; // getting message timeout

    _wakeCommand = wakeCommand;

    _recognizer = nullptr;
    _stream = nullptr;
    _lastActivityTime = std::chrono::steady_clock::now();
    _audioBuffer.reserve(_framesPerBuffer * 2);  // Pre-allocate buffer
    _processBuffer.clear();
}


SpeechRecognizingOperator::~SpeechRecognizingOperator() {
    close();
}

// Voice Activity Detection
bool SpeechRecognizingOperator::is_human_speech_hybrid(const short* audio_data, size_t length) {
    // 1. Energy check (hızlı)
    long long energy = 0;
    for (size_t i = 0; i < length; i++) {
        energy += abs(audio_data[i]);
    }
    double avg_energy = static_cast<double>(energy) / length;
    if (avg_energy < _silenceThreshold) {
        return false;  // Çok sessiz
    }else {
        return true;
    }
}

// Check for wake word in recognition result
bool SpeechRecognizingOperator::contains_wake_word(const std::string& text) {
    std::string lower_text = text;
    std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
    return lower_text.find(_wakeCommand) != std::string::npos;
}


int SpeechRecognizingOperator::paCallback(const void *input, void *output,
                                            unsigned long frameCount,
                                            const PaStreamCallbackTimeInfo *timeInfo,
                                            PaStreamCallbackFlags statusFlags,
                                            void *userData) {
    auto *controller = static_cast<SpeechRecognizingOperator *>(userData);
    const short* audio_data = static_cast<const short*>(input);
    static bool talking = false;
    static std::chrono::steady_clock::time_point lastTalkingFrameTime;

    if (_isSpeaking.load()) {
        return paContinue;
    }

    bool has_voice = controller->is_human_speech_hybrid(audio_data, frameCount);

    if (has_voice) {
        talking = true;
        controller->_processBuffer.insert(
        controller->_processBuffer.end(),
            audio_data,
            audio_data + frameCount
        );
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        lastTalkingFrameTime = now;  // Son ses çerçevesinin zamanını güncelle
    }else {
        if (talking) {
            controller->_processBuffer.insert(
        controller->_processBuffer.end(),
            audio_data,
            audio_data + frameCount
            );
        }

        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

        if (talking && (now - lastTalkingFrameTime > std::chrono::seconds(1))) {
            std::unique_lock<std::mutex> lock(controller->_bufferMutex);

            controller->_bufferQueue.push_back(controller->_processBuffer);

            controller->_processBuffer.clear();
            talking = false;
        }

    }

    return paContinue;
}

void SpeechRecognizingOperator::load_model(const std::string &modelPath) {
    _model = vosk_model_new(modelPath.c_str());
    _recognizer = vosk_recognizer_new(_model, _sampleRate);

    // _recognizer = vosk_recognizer_new_grm(
    // _model,
    // _sampleRate,
    // "[\"metafoniks\", \"iğrenç\", \"seven eight nine zero\", \"[unk]\"]");

    vosk_recognizer_set_max_alternatives(_recognizer, 3);  // Only best result
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
                               _sampleRate,
                               _framesPerBuffer,
                               paClipOff,      // No clipping
                               paCallback,
                               this);

    if (err != paNoError) {
        ERROR("Failed to open audio stream: {}", std::string(Pa_GetErrorText(err)));
        return false;
    }

    return true;
}

bool SpeechRecognizingOperator::start_listen() {
    if (Pa_StartStream(_stream) != paNoError) {
        ERROR("Failed to start audio stream.");
        return false;
    }

    return true;
}

void SpeechRecognizingOperator::stop_listen() {
    Pa_StopStream(_stream);
}


std::vector<int16_t> SpeechRecognizingOperator::get_buffer() {
    std::unique_lock<std::mutex> lock(_bufferMutex);

    if (!_bufferQueue.empty()) {
        std::vector<int16_t> audioData(1500, 0);  // prepare silence at beginning
        audioData.insert(audioData.end(), _bufferQueue.front().begin(), _bufferQueue.front().end());
        _bufferQueue.pop_front();
        return audioData;
    }else {
        return std::vector<int16_t>();
    }

}

std::string SpeechRecognizingOperator::get_text(const std::vector<int16_t> &audioData) {

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    _isSpeaking.store(true);
    std::string text;
    std::string result;

    int final = vosk_recognizer_accept_waveform(_recognizer, (const char*)audioData.data(), audioData.size() * sizeof(int16_t));
    if (final) {
        result = vosk_recognizer_result(_recognizer);
        INFO("res:  {}", result);
        json j = json::parse(result);
        if (j.contains("alternatives")) {
            text = j["alternatives"][0]["text"];
        }
    } else {
        result = vosk_recognizer_partial_result(_recognizer);
        INFO("res:  {}", result);

        json j = json::parse(result);
        if (j.contains("partial")) {
            text = j["partial"];
        }
    }
    _isSpeaking.store(false);
    vosk_recognizer_reset(_recognizer);

    return text;
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

    if (_model) {
        vosk_model_free(_model);
    }
}

