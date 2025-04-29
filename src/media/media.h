#ifndef __MEDIA_H__
#define __MEDIA_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "h264.h"

#define H264_STREAM 0
#define H265_STREAM 1
#define ACC_StREAM  3

typedef struct
{
    void *media;
    void *(*createMedia)(Buffer *buffer);
    void (*destroyMedia)(void *media);
    Buffer *(*getMediaFrame)(void *media, int index);
    Buffer *(*getMediaInfo)(void *media);
    void *(*getMediaConfig)(void *media);
} MediaFunc;

typedef struct 
{
    int type;
    char app[64];
    MediaFunc func;
} Media;

static inline int initStreamType(int stream_type, MediaFunc *func)
{
    if (stream_type == H264_STREAM) {
        func->createMedia   = createH264Media;
        func->destroyMedia  = destroyH264Media;
        func->getMediaFrame = getH264MediaFrame;
        func->getMediaInfo  = getH264MediaAVC;
        func->getMediaConfig = getH264MediaConfig;
        return NET_SUCCESS;
    }
    return NET_FAIL;
}

Media *createMediaChannl(const char *app, int stream_type, const char *file_path);
void destroyMediaChannl(Media *media);

Buffer *getMediaFrame(Media *media, int index);
Buffer *getMediaInfo(Media *media);
void *getMediaConfig(Media *media);

#endif // !__MEDIA_H__




