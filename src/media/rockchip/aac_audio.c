#include "aac_audio.h"
#include <faac.h>
#include <alsa/asoundlib.h>

typedef struct 
{
    unsigned long u64PcmSampleRate;  // 采样率
    unsigned int  u32PcmSampleBits; // 采样位数
	unsigned int  u32PcmChannels;   // 声道数
    unsigned long u64PcmInSampleCnt; // 打开编码器时传出的参数，编码传入的PCM采样数（不是字节）
	unsigned long u64AacOutMaxBytes; // 打开编码器时传出的参数，编码传出最大字节数
    snd_pcm_t *capture_handle;
    faacEncHandle  pFaacEncHandle;
} AacAudio;

AudioMedia *createAlsaAacMedia(const char *file)
{
    AudioMedia *media = CALLOC(1, AudioMedia);
    if (!media)
        return NULL;

    AacAudio *aac_encode = CALLOC(1, AacAudio);
    if (!media)
        return NULL;

    aac_encode->u64PcmSampleRate = 16000;
    aac_encode->u32PcmSampleBits = 16;
    aac_encode->u32PcmChannels   = 2;

    //plughw:3,0
    aac_encode->capture_handle = createAlsaAudio("plughw:2,0", aac_encode->u64PcmSampleRate, aac_encode->u32PcmChannels, SND_PCM_FORMAT_S16_LE);
    if (!aac_encode->capture_handle)
    {
        ERR("open alsa error");
        return NULL;
    }

    media->aac_context = aac_encode;

	faacEncConfigurationPtr pFaacEncConf = NULL;
    aac_encode->pFaacEncHandle = faacEncOpen(aac_encode->u64PcmSampleRate, aac_encode->u32PcmChannels, &aac_encode->u64PcmInSampleCnt, &aac_encode->u64AacOutMaxBytes);

    pFaacEncConf = faacEncGetCurrentConfiguration(aac_encode->pFaacEncHandle);
    pFaacEncConf->inputFormat = FAAC_INPUT_16BIT;

    LOG("-----%ld--%ld-", aac_encode->u64PcmInSampleCnt, aac_encode->u64AacOutMaxBytes);

#if 0
	/* 下面参数不用设置，保存默认即可 */
	pFaacEncConf->aacObjectType = LOW; 	// MAIN:1  LOW:2  SSR:3  LTP:4
	pFaacEncConf->mpegVersion = MPEG4; 	// MPEG2:0  MPEG4:1
	pFaacEncConf->useTns = 1; 			/* Use Temporal Noise Shaping */
	pFaacEncConf->shortctl = 0; 		// SHORTCTL_NORMAL:0  SHORTCTL_NOSHORT:1  SHORTCTL_NOLONG:2
	pFaacEncConf->allowMidside = 1; 	/* Allow mid/side coding */
	pFaacEncConf->quantqual = 0; 		/* Quantizer quality */
	pFaacEncConf->outputFormat = 1; 	// 0:Raw  1:ADTS
	pFaacEncConf->bandWidth = 32000;//0 /* AAC file frequency bandwidth */
	pFaacEncConf->bitRate = 48000;//0 	/* bitrate / channel of AAC file */
#endif

	// 设置编码器配置 c/c: 重新设置编码器的配置信息
	faacEncSetConfiguration(aac_encode->pFaacEncHandle, pFaacEncConf);
    LOG("aac_encode->capture_handle %p", aac_encode->capture_handle);
    return media;
}

void destroyAlsaAacMedia(AudioMedia *media)
{
    if (!media)
        return;

    AacAudio *aac_encode = (AacAudio  *)media->aac_context;
    faacEncClose(aac_encode->pFaacEncHandle);
    FREE(media->aac_context);
    FREE(media);
}


Buffer *getAlsaAacMediaFrame(AudioMedia *media)
{
    AacAudio *aac_encode = (AacAudio  *)media->aac_context;
    readAlsaAudio(aac_encode->capture_handle, 2, aac_encode->u64PcmInSampleCnt/2);
    // Buffer *buffer = createBuffer(aac_encode->u64PcmInSampleCnt * 2 * 2);
    // if (!buffer)
    // {
    //     ERR("buffer error");
    //     return NULL;
    // }

    // LOG("aac_encode->u64PcmInSampleCnt %d",aac_encode->u64PcmInSampleCnt);
    // LOG("aac_encode->u32PcmChannel %d",aac_encode->u32PcmChannels);
    // LOG("aac_encode->u64PcmInSampleCnt/aac_encode->u32PcmChannels %d", aac_encode->u64PcmInSampleCnt/aac_encode->u32PcmChannels);
    // LOG("aac_encode->capture_handle %p", aac_encode->capture_handle);
    // snd_pcm_uframes_t frame_size = aac_encode->u64PcmInSampleCnt/aac_encode->u32PcmChannels;
    // int32_t count = snd_pcm_readi(aac_encode->capture_handle, buffer->data, frame_size);
    // LOG("coount %d",count);
    // unsigned char *pu8AacEncBuf  = NULL;
    // pu8AacEncBuf = (unsigned char*)malloc(aac_encode->u64AacOutMaxBytes);
 
	// int s32EncAacBytes = faacEncEncode(aac_encode->pFaacEncHandle, (int32_t*)buffer->data, count, 
	// 												pu8AacEncBuf, (unsigned int)aac_encode->u64AacOutMaxBytes);
    return NULL;
}

