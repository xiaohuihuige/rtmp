#ifndef __H264_H__
#define __H264_H__

#include <schedule/net-common.h>
#include "type.h"
#include "h264_sps.h"

typedef struct 
{
    Buffer *pps;
    Buffer *sps;
} H264Info;

typedef struct 
{
    FifoQueue *frame_fifo;
    Buffer *avc_buffer;
    int frame_count;
    sps_t *sps;
} H264Media;

void *createH264Media(Buffer *buffer);
void destroyH264Media(void *media);
Buffer *getH264MediaFrame(void *media, int index);
Buffer *getH264MediaAVC(void *media);
void *getH264MediaConfig(void *media);

#endif // !__H264_H__
