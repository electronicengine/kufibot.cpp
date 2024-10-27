#include "speech_process_controller.h"

SpeechProcessController* SpeechProcessController::_instance = nullptr;

SpeechProcessController* SpeechProcessController::get_instance() {
    if (_instance == nullptr) {
        _instance = new SpeechProcessController();
    }
    return _instance;
}

SpeechProcessController::SpeechProcessController() {}

void SpeechProcessController::loadModel(const std::string& modelPath, const std::string& modelConfigPath){
    std::optional<piper::SpeakerId> speakerId = 0;
    piper::initialize(piperConfig);
    loadVoice(piperConfig, modelPath, modelConfigPath,
              voice, speakerId, false);
    std::cout << "Voice loaded successfully." << std::endl;
}

void SpeechProcessController::speakText(const std::string &text)
{
    synthesizeText(text);
    playAudio();
}

void SpeechProcessController::synthesizeText(const std::string& text) {
    piper::SynthesisResult result;
    auto audioCallback = [this]() { audioCallbackFunc(); };
    piper::textToAudio(piperConfig, voice, text, audioBuffer, result, audioCallback);
}

void SpeechProcessController::playAudio() {
    snd_pcm_t* pcmHandle;
    snd_pcm_hw_params_t* params;
    int err;

    if ((err = snd_pcm_open(&pcmHandle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        std::cout << "Error opening PCM device: " << err << std::endl;
        return;
    }

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcmHandle, params);
    snd_pcm_hw_params_set_access(pcmHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcmHandle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcmHandle, params, 1);
    unsigned int sampleRate = 22050;
    snd_pcm_hw_params_set_rate_near(pcmHandle, params, &sampleRate, 0);
    if ((err = snd_pcm_hw_params(pcmHandle, params)) < 0) {
        std::cout << "Error setting PCM parameters: " << err << std::endl;
        snd_pcm_close(pcmHandle);
        return;
    }

    std::vector<int16_t> internalBuffer;
    {
        std::unique_lock lock(mutAudio);
        cvAudio.wait(lock, [this] { return audioReady; });
        if (!audioFinished) {
            copy(sharedAudioBuffer.begin(), sharedAudioBuffer.end(), back_inserter(internalBuffer));
            sharedAudioBuffer.clear();
            audioReady = false;
        }
    }

    if ((err = snd_pcm_writei(pcmHandle, internalBuffer.data(), internalBuffer.size())) < 0) {
        snd_pcm_prepare(pcmHandle);
        std::cout << "Audio underrun: " << err << std::endl;
    }

    snd_pcm_drain(pcmHandle);
    snd_pcm_close(pcmHandle);
}

void SpeechProcessController::audioCallbackFunc() {
    std::unique_lock lock(mutAudio);
    copy(audioBuffer.begin(), audioBuffer.end(), back_inserter(sharedAudioBuffer));
    audioReady = true;
    cvAudio.notify_one();
}

SpeechProcessController::~SpeechProcessController() {
    piper::terminate(piperConfig);
}