#ifndef __RTMP_SERVER_H__
#define __RTMP_SERVER_H__
#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>
#include "rtmp_media.h"

typedef struct 
{
   TcpServer *server;
   FifoQueue *stream;
   Mutex myMutex;
} RtmpServer;

RtmpServer *createRtmpServer(const char *ip, int port);
void destroyRtmpServer(RtmpServer *rtmp);

void addMediaToRtmpServer(RtmpServer *rtmp, RtmpMedia *media);
void removMeidaByRtmpServer(RtmpServer *rtmp, RtmpMedia *media);
RtmpMedia *findMediaByRtmpServer(RtmpServer *rtmp, const char *app);

#endif