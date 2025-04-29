#ifndef __RTMP_SESSION_H__
#define __RTMP_SESSION_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "type.h"
#include "media.h"

typedef struct 
{
   int cache;
   int index;
   int interval;
   int base_time;
   int state;
   Seesion *conn;
   RtmpPacket *packet;
   SessionConfig config;
   TaskTimer *stream_task;
   Buffer *buffer;
   bs_t *b;
   Media *media;
} RtmpSession;

RtmpSession *createRtmpSession(Seesion *conn);
void destroyRtmpSession(RtmpSession *session);
void recvRtmpSession(RtmpSession *session, Buffer *buffer);

int startPushStreamTask(RtmpSession *session);
int findMediaStreamChannl(RtmpSession *session);
int stopPushStreamTask(RtmpSession *session);

#endif