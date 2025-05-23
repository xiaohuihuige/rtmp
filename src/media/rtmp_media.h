#ifndef __MEDIA_H__
#define __MEDIA_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "h264.h"
#include "rtmp_session.h"
#include "type.h"
#include "util.h"

RtmpMedia *createRtmpMedia(RtmpConfig *config);
void destroyRtmpMedia(RtmpMedia *media);

int findRtmpMedia(RtmpSession *session);
int startPushSessionStream(RtmpSession *session);
void stopPushSessionstream(RtmpSession *session);

#endif // !__MEDIA_H__




