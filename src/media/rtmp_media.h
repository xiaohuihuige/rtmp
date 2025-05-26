#ifndef __MEDIA_H__
#define __MEDIA_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "rtmp_session.h"
#include "type.h"
#include "util.h"


RtmpConfig *createFileRtmpConfig(const char *app, const char *h264_file, const char *aac_file);

RtmpMedia *createRtmpMedia(RtmpConfig *config);
void destroyRtmpMedia(RtmpMedia *media);

int findRtmpMedia(RtmpSession *session);
void addRtmpSessionToMedia(RtmpMedia *media, RtmpSession *session);
void removeRtmpSession(RtmpMedia *media, RtmpSession *session);

#endif // !__MEDIA_H__




