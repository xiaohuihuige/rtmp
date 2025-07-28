#include "media_config.h"
#include "aac.h"
#include "h264.h"
#include "mpp_h264.h"

RtmpConfig *createRtmpConfig(const char *app, const char *h264_file, const char *aac_file,
                             VideoMedia *(*createH264Stream)(const char *file), 
                             void (*destroyH264Stream)(VideoMedia *media),
                             Buffer *(*getH264Stream)(VideoMedia *media), 
                             AudioMedia *(*createAacStream)(const char *file),
                             void (*destroyAacStream)(AudioMedia *meida), 
                             Buffer *(*getAacStream)(AudioMedia *media))
{
    RtmpConfig *config = CALLOC(1, RtmpConfig);
    if (!config)
        return NULL;

    config->app = app;

    config->aac_file = aac_file;
    config->h264_file = h264_file;

    config->createH264Stream = createH264Stream;
    config->destroyH264Stream = destroyH264Stream;
    config->getH264Stream = getH264Stream;

    config->createAacStream = createAacStream;
    config->destroyAacStream = destroyAacStream;
    config->getAacStream = getAacStream;

    return config;
}

void destroyRtmpConfig(RtmpConfig *config)
{
    FREE(config);
}

RtmpConfig *createFileRtmpConfig(const char *app, const char *h264_file, const char *aac_file)
{
    return createRtmpConfig(app, h264_file, aac_file,
                            createH264Media, destroyH264Media, getH264MediaFrame,
                            createAacMedia, destroyAacMedia, getAacMediaFrame);
}

RtmpConfig *createOnlieRtmpConfig(const char *app, const char *v4l2_device, const char *aac_device)
{
    return createRtmpConfig(app, v4l2_device,  aac_device,
                            createMppH264Media, destroyMppH264Media, getMppH264MediaFrame,
                            createAacMedia, destroyAacMedia, getAacMediaFrame);
}