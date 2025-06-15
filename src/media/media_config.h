#ifndef __MEDIA_CONFIG_H__
#define __MEDIA_CONFIG_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "type.h"
#include "util.h"

//本地文件流，一次性读文件，对内存消耗会比较大
RtmpConfig *createFileRtmpConfig(const char *app, const char *h264_file, const char *aac_file);

//在线直播流
RtmpConfig *createOnlieRtmpConfig(const char *app, const char *v4l2_device, const char *aac_device);

void destroyRtmpConfig(RtmpConfig *config);

#endif //  __MEDIA_CONFIG_H__
