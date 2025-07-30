#include "aac_audio.h"
#include <faac.h>
#include <alsa/asoundlib.h>
#include "alsa_read_pcm.h"
#include "util.h"
#include "send_chunk.h"


AudioMedia *createAlsaAacMedia(RtmpConfig *config)
{
    if (!config)
        return NULL;

    AudioMedia *media = CALLOC(1, AudioMedia);
    if (!media)
        return NULL;

    media->u32PcmChannels   = config->u32PcmChannels;
    media->u32PcmSampleBits = config->u32PcmSampleBits;
    media->u64PcmSampleRate = config->u64PcmSampleRate;
    media->stereo           = config->u32PcmChannels - 1;
    media->audiocodecid     = AUDIOCODECID;
    media->audiodatarate    = AUDIODATARATE;
    media->audiosamplerate  = config->u64PcmSampleRate;
    media->audiosamplesize  = config->u32PcmSampleBits;

    do {

        media->adts_sequence = rtmpadtsSequence(LOW , 8, 1, media->stereo);
        if (!media->adts_sequence)
            break;

        //plughw:3,0
        media->capture_handle = createAlsaAudio(config->alsa_device, media->u64PcmSampleRate, media->u32PcmChannels, SND_PCM_FORMAT_S16_LE);
        if (!media->capture_handle)
            break;

        media->pFaacEncHandle = faacEncOpen(media->u64PcmSampleRate, media->u32PcmChannels, &media->u64PcmInSampleCnt, &media->u64AacOutMaxBytes);
        if(!media->pFaacEncHandle)
            break;

        media->pFaacEncConf = faacEncGetCurrentConfiguration(media->pFaacEncHandle);
        media->pFaacEncConf->inputFormat    = FAAC_INPUT_16BIT;
        media->pFaacEncConf->aacObjectType  = LOW ;
        media->pFaacEncConf->outputFormat   = 0;
        faacEncSetConfiguration(media->pFaacEncHandle, media->pFaacEncConf);

        media->duration = (int)(1000 * media->u64PcmInSampleCnt/media->u32PcmChannels)/media->audiosamplerate;

        LOG("success %p", media);
        return media;
    } while(0);

    destroyAlsaAacMedia(media);

    return NULL;
}

void destroyAlsaAacMedia(AudioMedia *media)
{
    if (!media)
        return;
    if (media->capture_handle)
        snd_pcm_close(media->capture_handle);
    if (media->pFaacEncHandle)    
        faacEncClose(media->pFaacEncHandle);
    FREE(media);
}

Buffer *getAlsaAacMediaFrame(AudioMedia *media)
{
    Buffer *buffer = readAlsaAudio(media->capture_handle, media->u32PcmChannels, media->u64PcmInSampleCnt/media->u32PcmChannels);
    if (!buffer)
        return NULL;

    Buffer *aac_buffer = createBuffer(media->u64AacOutMaxBytes);
    if (!aac_buffer)
    {
        FREE(buffer);
        return NULL;
    }

    int s32EncAacBytes = faacEncEncode(media->pFaacEncHandle, 
                                     (int32_t*)buffer->data, 
                                     media->u64PcmInSampleCnt, 
									 aac_buffer->data, 
                                     media->u64AacOutMaxBytes);

    FREE(buffer);

    if (s32EncAacBytes > 0)
    {
        aac_buffer->timestamp = calculateTimeStamp(&media->fractional_part, media->u64PcmSampleRate, media->u64PcmInSampleCnt/media->u32PcmChannels);
        aac_buffer->length = s32EncAacBytes;
        Buffer *aac_rtmp_buffer =rtmpWriteAudioFrame(aac_buffer, 8, 1, media->stereo);
        FREE(aac_buffer);
        // AdtsHeader header = {0};
        // paresADTSHeader(&header, aac_buffer->data, s32EncAacBytes);
        return aac_rtmp_buffer;
    } 
    return NULL;
}

