#ifndef __RTMP_SESSION_H__
#define __RTMP_SESSION_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "type.h"

typedef struct 
{
   int state;
   Seesion *conn;
   RtmpPacket *packet;
   SessionConfig config;
   TaskTimer *stream_task;
   Buffer *buffer;
   bs_t *b;
   RtmpMedia *media;
   MediaChannle channle[2];
} RtmpSession;

RtmpSession *createRtmpSession(Seesion *conn);
void destroyRtmpSession(RtmpSession *session);
void recvRtmpSession(RtmpSession *session, Buffer *buffer);

#endif