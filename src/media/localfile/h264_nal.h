#ifndef __H264_NAL_H__
#define __H264_NAL_H__

#include <schedule/net-common.h>

int find_nal_unit(uint8_t *buf, int size, int *nal_start, int *nal_end);
int rbsp_to_nal(const uint8_t* rbsp_buf, const int* rbsp_size, uint8_t* nal_buf, int* nal_size);
int nal_to_rbsp(const uint8_t* nal_buf, int* nal_size, uint8_t* rbsp_buf, int* rbsp_size);

#endif
