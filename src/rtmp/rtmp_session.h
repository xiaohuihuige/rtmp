#ifndef __RTMP_SESSION_H__
#define __RTMP_SESSION_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "type.h"
#include "media.h"

typedef struct 
{
   int interval;
   int base_time;
   int state;
   Seesion *conn;
   RtmpPacket *packet;
   SessionConfig config;
   TaskTimer *stream_task;
   Media *media;
   Buffer *buffer;
   bs_t *b;
} RtmpSession;

RtmpSession *createRtmpSession(Seesion *conn);
void destroyRtmpSession(RtmpSession *session);
void recvRtmpSession(RtmpSession *session, Buffer *buffer);
int addRtmpPushTask(RtmpSession *session);

#endif