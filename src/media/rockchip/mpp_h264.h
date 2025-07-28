#ifndef __MPP_H264_H__
#define __MPP_H264_H__

#include <schedule/net-common.h>
#include "type.h"
#include "h264_sps.h"

VideoMedia *createMppH264Media(const char *device_name);
void destroyMppH264Media(VideoMedia *media);
Buffer *getMppH264MediaFrame(VideoMedia *media);

#endif