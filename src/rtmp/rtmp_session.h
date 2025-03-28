#ifndef __RTMP_SESSION_H__
#define __RTMP_SESSION_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>

typedef struct 
{
   int fd;
   int state;
   Seesion *conn;
} RtmpSession;

RtmpSession *createRtmpSession(Seesion *conn);
void destroyRtmpSession(RtmpSession *session);
void recvRtmpSession(RtmpSession *session, Buffer *buffer);

#endif