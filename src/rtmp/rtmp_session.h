#ifndef __RTMP_SESSION_H__
#define __RTMP_SESSION_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "type.h"
#include <schedule/fifo_queue.h>

typedef struct 
{
   int state;
   Seesion *conn;
   SessionConfig config;
   Buffer *buffer;
   Buffer *temp_buffer;
   bs_t *b;
   RtmpMedia *media;
   MediaChannle channle[2];
   Mutex myMutex;
   TaskTimer *pull_stream_timer;
   Queue *queue;
} RtmpSession;

RtmpSession *createRtmpSession(Seesion *conn);
void destroyRtmpSession(RtmpSession *session);
void recvRtmpSession(RtmpSession *session, Buffer *buffer);

int createSessionStreamTimer(RtmpSession *session);

#endif