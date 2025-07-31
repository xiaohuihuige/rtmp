#ifndef __H264_H__
#define __H264_H__

#include <schedule/net-common.h>
#include "type.h"
#include "h264_sps.h"

VideoMedia *createH264Media(RtmpConfig *config);
void destroyH264Media(VideoMedia *media);
Buffer *getH264MediaFrame(VideoMedia *media);

#endif // !__H264_H__
