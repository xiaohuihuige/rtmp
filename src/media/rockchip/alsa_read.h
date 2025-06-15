#ifndef __ALSA_H__
#define __ALSA_H__

#include <alsa/asoundlib.h>
#include <schedule/net-common.h>
#include <schedule/buffer.h>

typedef struct 
{
    snd_pcm_t *capture_handle_;
    uint32_t sample_rate_;
    uint32_t channel_count_;
} Alsa;

//plughw:3,0
Alsa *createAlsaAudio(const char *device_name, int sample_rate);
Buffer *readAlsaAudio(Alsa *alsa, int32_t num_samples);
void destroyAlsaAudio(Alsa *alsa);

#endif
