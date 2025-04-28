#ifndef __H264_H__
#define __H264_H__

#include <schedule/net-common.h>
#include "type.h"

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
} H264Media;

H264Media *createH264Media(Buffer *buffer);
void destroyH264Media(H264Media *media);
Buffer *getH264MediaFrame(H264Media *media, int index);
Buffer *getH264MediaAVC(H264Media *media);

#endif // !__H264_H__
