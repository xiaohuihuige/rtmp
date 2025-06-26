#ifndef __RTMP_EVENT_H__
#define __RTMP_EVENT_H__
#include "rtmp_session.h"
#include "type.h"

int handleRtmpEvent(RtmpSession *session, RtmpPacket *packet);

#endif // !__RTMP_EVENT_H__
