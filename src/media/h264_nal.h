#ifndef __H264_NAL_H__
#define __H264_NAL_H__

#include <schedule/net-common.h>

int find_nal_unit(uint8_t *buf, int size, int *nal_start, int *nal_end);

#endif
