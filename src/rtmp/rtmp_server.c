#include "rtmp_server.h"

static void _recvMessage(void *args, void *buffer)
{
    LOG("recv size:%d, %p, %s", ((Buffer *)buffer)->length, args, ((Buffer *)buffer)->data);

    char *message = "Hello, server!";
    send(((Seesion *)args)->fd, message, strlen(message), 0);
}

RtmpServer *createRtmpServer(const char *ip, int port)
{
    if (!ip)
        return NULL;

    RtmpServer *rtmp = CALLOC(1, RtmpServer);
    if (!rtmp )
        return NULL;

    rtmp->server = createTcpServer(ip, port);

    setTcpServerCallBack(rtmp->server, NULL, _recvMessage, NULL);

    return rtmp;
}

void destroyRtmpServer(RtmpServer *rtmp)
{
    if (!rtmp)
        return;

    destroyTcpServer(rtmp->server);
}