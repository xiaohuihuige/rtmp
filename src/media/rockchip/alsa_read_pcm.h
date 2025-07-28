#ifndef __ALSA_H__
#define __ALSA_H__

#include <alsa/asoundlib.h>
#include <schedule/net-common.h>
#include <schedule/buffer.h>

//plughw:3,0
snd_pcm_t *createAlsaAudio(const char *device_name, int sample_rate, int channel_count, snd_pcm_format_t format);
Buffer *readAlsaAudio(snd_pcm_t *capture_handle, int channel_count, int32_t num_samples);
#endif
