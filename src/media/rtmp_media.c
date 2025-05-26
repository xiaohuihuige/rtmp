#include "rtmp_media.h"
#include "send_chunk.h"
#include "rtmp_server.h"
#include "aac.h"
#include "h264.h"

RtmpConfig *createRtmpConfig(const char *app, const char *h264_file, const char *aac_file, 
                            CreateH264Stream createH264Stream, DestroyH264Stream destroyH264Stream,
                            GetH264Stream getH264Stream, CreateAacStream createAacStream, 
                            DestroyAacStream destroyAacStream, GetAacStream getAacStream)
{

    RtmpConfig *config = CALLOC(1, RtmpConfig);
    if (!config)
        return NULL;

    config->app = app;

    config->aac_file  = aac_file;
    config->h264_file = h264_file;

    config->createH264Stream  = createH264Stream;
    config->destroyH264Stream = destroyH264Stream;
    config->getH264Stream     = getH264Stream;

    config->createAacStream   = createAacStream;
    config->destroyAacStream  = destroyAacStream;
    config->getAacStream      = getAacStream;

    return config;
}

RtmpConfig *createFileRtmpConfig(const char *app, const char *h264_file, const char *aac_file)
{
    return createRtmpConfig(app, h264_file, aac_file, 
                        createH264Media, destroyH264Media, getH264MediaFrame, 
                        createAacMedia,  destroyAacMedia, getAacMediaFrame);
}

RtmpMedia *createRtmpMedia(RtmpConfig *config)
{   
    if (!config)
        return NULL;

    RtmpMedia *media = CALLOC(1, RtmpMedia);
    if (!media)
        return NULL;

    snprintf(media->app, sizeof(media->app), "%s", config->app);

    media->config = config;

    if (config->createH264Stream)
        media->video = config->createH264Stream(config->h264_file);

    if (config->createAacStream)
        media->audio = config->createAacStream(config->aac_file);

    return media;
}

void destroyRtmpMedia(RtmpMedia *media)
{
    if (media->video && media->config && media->config->destroyH264Stream)
        media->config->destroyH264Stream(media->video);

    if (media->audio && media->config && media->config->destroyAacStream)
        media->config->destroyAacStream(media->audio);

    FREE(media->config);

    media->audio  = NULL;
    media->video  = NULL;
    media->config = NULL;

    FREE(media);
}