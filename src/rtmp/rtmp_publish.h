#ifndef __RTMP_PUBLISH_H__
#define __RTMP_PUBLISH_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "rtmp_session.h"
#include "type.h"
#include "util.h"

int findRtmpMedia(RtmpSession *session);
int startPushSessionStream(RtmpSession *session);
void stopPushSessionstream(RtmpSession *session);

#endif