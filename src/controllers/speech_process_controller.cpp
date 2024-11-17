#include "speech_process_controller.h"
#include "../ui/main_window.h"

SpeechProcessController* SpeechProcessController::_instance = nullptr;

SpeechProcessController* SpeechProcessController::get_instance() {
    if (_instance == nullptr) {
        _instance = new SpeechProcessController();
    }
    return _instance;
}

SpeechProcessController::SpeechProcessController() {
    mpg123_init();  // Initialize MPG123
    mh = mpg123_new(nullptr, &mpg123_err);
    if (mpg123_err != MPG123_OK) {
        MainWindow::log("Failed to initialize mpg123: " + std::string(mpg123_plain_strerror(mpg123_err)), LogLevel::LOG_ERROR);
    }
}

void SpeechProcessController::loadModel(const std::string& modelPath, const std::string& modelConfigPath){
    std::optional<piper::SpeakerId> speakerId = 0;
    piper::initialize(piperConfig);
    loadVoice(piperConfig, modelPath, modelConfigPath,
              voice, speakerId, false);
    MainWindow::log( "Voice loaded successfully.", LogLevel::LOG_INFO);

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
        MainWindow::log("Error opening PCM device: " + std::string(mpg123_plain_strerror(err)), LogLevel::LOG_ERROR);

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
        MainWindow::log("Error setting PCM parameters: " + std::string(mpg123_plain_strerror(err)), LogLevel::LOG_ERROR);
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
        MainWindow::log("Error Audio underrun: " + std::string(mpg123_plain_strerror(err)), LogLevel::LOG_ERROR);
    }

    snd_pcm_drain(pcmHandle);
    snd_pcm_close(pcmHandle);
}


void SpeechProcessController::playMusic(const std::string& mp3_file) {
    if (mpg123_open(mh, mp3_file.c_str()) != MPG123_OK) {
        MainWindow::log("Error opening MP3 file: " + mp3_file, LogLevel::LOG_ERROR);
        return;
    }

    unsigned int rate;
    int channels, encoding;
    mpg123_getformat(mh, reinterpret_cast<long*>(&rate), &channels, &encoding);

    snd_pcm_t* pcmHandle;
    snd_pcm_hw_params_t* params;
    int err;

    if ((err = snd_pcm_open(&pcmHandle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        MainWindow::log("Error opening PCM device: " + std::to_string(err), LogLevel::LOG_ERROR);
        return;
    }

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcmHandle, params);
    snd_pcm_hw_params_set_access(pcmHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcmHandle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcmHandle, params, channels);
    snd_pcm_hw_params_set_rate_near(pcmHandle, params, &rate, 0);
    snd_pcm_hw_params(pcmHandle, params);
    snd_pcm_prepare(pcmHandle);

    size_t buffer_size = mpg123_outblock(mh);
    unsigned char* buffer = new unsigned char[buffer_size];
    size_t done;

    while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK) {
        if (snd_pcm_writei(pcmHandle, buffer, done / 4) < 0) {
            snd_pcm_prepare(pcmHandle);  // Recover from underrun
        }
    }

    delete[] buffer;
    snd_pcm_drain(pcmHandle);
    snd_pcm_close(pcmHandle);
    mpg123_close(mh);

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