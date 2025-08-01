#include "speech_performing_operator.h"
#include "../logger.h"


SpeechPerformingOperator* SpeechPerformingOperator::_instance = nullptr;

SpeechPerformingOperator* SpeechPerformingOperator::get_instance() {
    if (_instance == nullptr) {
        _instance = new SpeechPerformingOperator();
    }
    return _instance;
}

SpeechPerformingOperator::SpeechPerformingOperator() {
    _piperConfig.eSpeakDataPath = "/usr/local/share/espeak-ng-data/";
    mpg123_init();  // Initialize MPG123
    _mh = mpg123_new(nullptr, &_mpg123Err);
    if (_mpg123Err != MPG123_OK) {
        ERROR("Failed to initialize mpg123: {}", std::string(mpg123_plain_strerror(_mpg123Err)));
    }
}

void SpeechPerformingOperator::loadModel(const std::string& modelPath){

    _modelPath = modelPath;
    _speakerId = 0;

    // stderr'ı geçici log dosyasına yönlendir
    int stderr_copy = dup(STDERR_FILENO);
    FILE* log_file = fopen("/tmp/so_logs.txt", "w");
    dup2(fileno(log_file), STDERR_FILENO);

    loadVoice(_piperConfig, _modelPath, _modelPath + ".json", _voice, _speakerId,false);
    piper::initialize(_piperConfig);

    // stderr'ı geri yükle
    dup2(stderr_copy, STDERR_FILENO);
    close(stderr_copy);
    fclose(log_file);

    // // Log dosyasını oku ve TRACE ile göster
    // std::ifstream logFile("/tmp/so_logs.txt");
    // std::string line;
    // while (std::getline(logFile, line)) {
    //     if (!line.empty()) {
    //         TRACE("SO: {}", line);
    //     }
    // }
    // logFile.close();
    unlink("/tmp/so_logs.txt"); // Geçici dosyayı sil
}

void SpeechPerformingOperator::speakText(const std::string &text)
{
    synthesizeText(text);
    playAudio();
}

void SpeechPerformingOperator::synthesizeText(const std::string& text) {
    piper::SynthesisResult result;
    auto audioCallback = [this]() { audioCallbackFunc(); };
    piper::textToAudio(_piperConfig, _voice, text, _audioBuffer, result, audioCallback);
}

void SpeechPerformingOperator::playAudio() {
    snd_pcm_t* pcmHandle;
    snd_pcm_hw_params_t* params;
    int err;

    if ((err = snd_pcm_open(&pcmHandle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        ERROR("Error opening PCM device: {} ", mpg123_plain_strerror(err));

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
        ERROR("Error setting PCM parameters: {} ", mpg123_plain_strerror(err));
        snd_pcm_close(pcmHandle);
        return;
    }

    std::vector<int16_t> internalBuffer;
    {
        std::unique_lock lock(_mutAudio);
        _cvAudio.wait(lock, [this] { return audioReady; });
        if (!audioFinished) {
            copy(_sharedAudioBuffer.begin(), _sharedAudioBuffer.end(), back_inserter(internalBuffer));
            _sharedAudioBuffer.clear();
            audioReady = false;
        }
    }

    if ((err = snd_pcm_writei(pcmHandle, internalBuffer.data(), internalBuffer.size())) < 0) {
        snd_pcm_prepare(pcmHandle);
        ERROR("Error Audio underrun: {}",  std::string(mpg123_plain_strerror(err)));
    }

    snd_pcm_drain(pcmHandle);
    snd_pcm_close(pcmHandle);
}


void SpeechPerformingOperator::playMusic(const std::string& mp3_file) {
    if (mpg123_open(_mh, mp3_file.c_str()) != MPG123_OK) {
        ERROR("Error opening MP3 file: {}" , mp3_file);
        return;
    }

    unsigned int rate;
    int channels, encoding;
    mpg123_getformat(_mh, reinterpret_cast<long*>(&rate), &channels, &encoding);

    snd_pcm_t* pcmHandle;
    snd_pcm_hw_params_t* params;
    int err;

    if ((err = snd_pcm_open(&pcmHandle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        ERROR("Error opening PCM device: {}" , std::to_string(err));
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

    size_t buffer_size = mpg123_outblock(_mh);
    unsigned char* buffer = new unsigned char[buffer_size];
    size_t done;

    while (mpg123_read(_mh, buffer, buffer_size, &done) == MPG123_OK) {
        if (snd_pcm_writei(pcmHandle, buffer, done / 4) < 0) {
            snd_pcm_prepare(pcmHandle);  // Recover from underrun
        }
    }

    delete[] buffer;
    snd_pcm_drain(pcmHandle);
    snd_pcm_close(pcmHandle);
    mpg123_close(_mh);

}



void SpeechPerformingOperator::audioCallbackFunc() {
    std::unique_lock lock(_mutAudio);
    copy(_audioBuffer.begin(), _audioBuffer.end(), back_inserter(_sharedAudioBuffer));
    audioReady = true;
    _cvAudio.notify_one();
}

SpeechPerformingOperator::~SpeechPerformingOperator() {
    piper::terminate(_piperConfig);
}
