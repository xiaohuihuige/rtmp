#ifndef __V4L2_CAPTURE_H__
#define __V4L2_CAPTURE_H__

#include <schedule/net-common.h>
#include "type.h"
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#define BUFFER_COUNT 4

typedef struct 
{
  unsigned char *start;
  int length;
} FrameBuf;

typedef struct 
{
    enum v4l2_buf_type type;
    struct v4l2_buffer buf;//图像数据
    int fd; 
    int bufcnt;
    FrameBuf mmap_buffer[BUFFER_COUNT];//图像数据虚拟地址
} V4l2Capture;

V4l2Capture *createV4l2Capture(const char *dev_name, int bufcnt, int width, int height, uint32_t format, int fps);
void destroyV4l2Capture(V4l2Capture *v4l2);
Buffer *getV4l2Frame(V4l2Capture *v4l2);

#endif