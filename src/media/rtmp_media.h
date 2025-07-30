#ifndef __MEDIA_H__
#define __MEDIA_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "gop_cache.h"
#include "rtmp_session.h"
#include "type.h"
#include "util.h"

RtmpMedia *createRtmpMedia(RtmpConfig *config);
void destroyRtmpMedia(RtmpMedia *media);

void addRtmpSessionToMedia(RtmpMedia *media, RtmpSession *session);
void removeRtmpSessionByMedia(RtmpMedia *media, RtmpSession *session);


static inline VideoMedia *getRtmpVideoMeida(RtmpMedia *media)
{
    if (!media || !media->video)
        return NULL;
    return media->video;
}

static inline AudioMedia *getRtmpAudioMedia(RtmpMedia *media)
{
    if (!media || !media->audio)
        return NULL;
    return media->audio;
}

static inline RtmpConfig *getRtmpConfig(RtmpMedia *media)
{
    if (!media || !media->config)
        return NULL;
    return media->config;
}

static inline VideoMedia *createVideoChannl(RtmpConfig *config)
{
    if (!config->createH264Stream)
        return NULL;

    return config->createH264Stream(config);
}

static inline AudioMedia *createAudioChannl(RtmpConfig *config)
{
    if (!config->createAacStream)
        return NULL;

    return config->createAacStream(config);
}

static inline void destroyVideoChannl(RtmpConfig *config, VideoMedia *video)
{
    if (!config || !video || !config->destroyH264Stream)
        return;
    return config->destroyH264Stream(video);
}

static inline void destroyAudioChannl(RtmpConfig *config, AudioMedia *audio)
{
    if (!config || !audio|| !config->destroyAacStream)
        return;
    return config->destroyAacStream(audio);
}

static inline Buffer *getH264Frame(RtmpConfig *config, VideoMedia *video)
{
    if (!config || !video|| !config->getH264Stream)
        return NULL;
    return config->getH264Stream(video);
}

static inline Buffer *getHAacFrame(RtmpConfig *config, AudioMedia *audio)
{
     if (!config || !audio|| !config->getAacStream)
        return NULL;
    return config->getAacStream(audio);
}

#endif // !__MEDIA_H__




