#ifndef __MEDIA_H__
#define __MEDIA_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>

#define H264_STREAM 0
#define H265_STRRAM 1

typedef struct 
{
    char app[64];
    Buffer *pps;
    Buffer *sps;
    FifoQueue *frame_fifo;
    Buffer *avc_buffer;
} Media;

Media *createMediaChannl(const char *app, int stream_type, const char *file_path);

#endif // !__MEDIA_H__




