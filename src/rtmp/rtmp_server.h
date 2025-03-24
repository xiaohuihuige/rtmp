#ifndef __RTMP_SERVER_H__
#define __RTMP_SERVER_H__
#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>

typedef struct 
{
   TcpServer * server;
} RtmpServer;

RtmpServer *createRtmpServer(const char *ip, int port);
void destroyRtmpServer(RtmpServer *rtmp);

#endif