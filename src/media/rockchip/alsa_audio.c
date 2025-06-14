#include "alsa_audio.h"

AudioMedia *createAlsaAacMedia(const char *file)
{
    // AudioMedia *media = CALLOC(1, AudioMedia);
    // if (!media)
    //     return NULL;

    // media->adts_sequence = rtmpadtsSequence(header.profile, header.samplingFreqIndex, 1, header.channelCfg);
    // if (!media->adts_sequence)
    //     return NULL;

    // media->stereo = header.channelCfg;
    // media->audiocodecid = AUDIOCODECID;
    // media->audiodatarate = AUDIODATARATE;
    // media->audiosamplerate = gSampleRateIndex[header.samplingFreqIndex];
    // media->audiosamplesize = 16;
    // media->duration = (int) (1024 * 1000)/media->audiosamplerate;

    return NULL;
}

void destroyAlsaAacMedia(AudioMedia *media)
{

}

Buffer *getAlsaAacMediaFrame(AudioMedia *media, int index)
{
    return NULL;
}

