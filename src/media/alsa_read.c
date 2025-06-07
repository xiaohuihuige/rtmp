
#include "alsa_read.h"

Alsa *createAlsaAudio(const char *device_name, int sample_rate)
{
    Alsa * alsa = CALLOC(1, Alsa);
    if (!alsa)
        return NULL;

    alsa->sample_rate_ = sample_rate;
    alsa->channel_count_ = 1;

    do {
        int32_t err = snd_pcm_open(&alsa->capture_handle_, device_name, SND_PCM_STREAM_CAPTURE, 0);
        if (err)
        {
            ERR("Unable to open: %s. %s", device_name, snd_strerror(err));
            break;
        }

        snd_pcm_hw_params_t *hw_params;
        snd_pcm_hw_params_alloca(&hw_params);

        err = snd_pcm_hw_params_any(alsa->capture_handle_, hw_params);
        if (err) {
            ERR("Failed to initialize hw_params: %s", snd_strerror(err));
            break;
        }

        err = snd_pcm_hw_params_set_access(alsa->capture_handle_, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
        if (err) {
            ERR("Failed to set access type: %s", snd_strerror(err));
            break;
        }

        err = snd_pcm_hw_params_set_format(alsa->capture_handle_, hw_params, SND_PCM_FORMAT_S16_LE);
        if (err) {
            ERR("Failed to set format: %s", snd_strerror(err));
            break;
        }

        // mono
        err = snd_pcm_hw_params_set_channels(alsa->capture_handle_, hw_params, 1);
        if (err) {
            ERR("Failed to set number of channels to 1. %s", snd_strerror(err));
            err = snd_pcm_hw_params_set_channels(alsa->capture_handle_, hw_params, 2);
            if (err) {
                ERR("Failed to set number of channels to 2. %s",snd_strerror(err));
                break;
            }

            alsa->channel_count_ = 2;
            ERR("Channel count is set to 2. Will use only 1 channel of it.");
        }

        int32_t dir = 0;
        err = snd_pcm_hw_params_set_rate_near(alsa->capture_handle_, hw_params, &alsa->sample_rate_, &dir);
        if (err) {
            ERR("Failed to set sample rate to, %d: %s", alsa->sample_rate_, snd_strerror(err));
            break;
        }

        err = snd_pcm_hw_params(alsa->capture_handle_, hw_params);
        if (err) {
            ERR("Failed to set hw params: %s", snd_strerror(err));
            break;
        }

        err = snd_pcm_prepare(alsa->capture_handle_);
        if (err) {
            ERR("Failed to prepare for recording: %s", snd_strerror(err));
            break;
        }

        LOG("Recording started!");

        return alsa;
    } while(0);

    destroyAlsaAudio(alsa);

    return NULL;
}

void destroyAlsaAudio(Alsa *alsa)
{
    LOG("destroyAlsaAudio %p", alsa);
    if (alsa && alsa->capture_handle_)
        snd_pcm_close(alsa->capture_handle_);
    FREE(alsa);
}

Buffer *readAlsaAudio(Alsa *alsa, int32_t num_samples)
{
    Buffer *buffer = createBuffer(num_samples * alsa->channel_count_ * 2);
    if (!buffer)
        return NULL;

    int32_t count = snd_pcm_readi(alsa->capture_handle_, buffer->data, num_samples);
    if (count == -EPIPE) {
        snd_pcm_prepare(alsa->capture_handle_);
        return NULL;
    } else if (count < 0)
    {
        ERR("Can't read PCM device: %s", snd_strerror(count));
        return NULL;
    }
    LOG("%d", buffer->length);
    return buffer;
}