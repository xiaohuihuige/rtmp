#include "rtmp_server.h"
#include "rtmp_session.h"


static void _recvMessage(void *seesion, Buffer *buffer)
{
    return recvRtmpSession((RtmpSession *)seesion, buffer);
}

static void *_createRtmpSession(Seesion *conn)
{
    return createRtmpSession(conn);
}

static void _destroyRtmpSession(void *seesion)
{
    return destroyRtmpSession((RtmpSession *)seesion);
}

RtmpServer *createRtmpServer(const char *ip, int port)
{
    if (!ip)
        return NULL;

    RtmpServer *rtmp = CALLOC(1, RtmpServer);
    if (!rtmp )
        return NULL;

    rtmp->server = createTcpServer(ip, port);

    setTcpServerCallBack(rtmp->server, _createRtmpSession, _recvMessage, _destroyRtmpSession);

    return rtmp;
}

void destroyRtmpServer(RtmpServer *rtmp)
{
    if (!rtmp)
        return;

    return destroyTcpServer(rtmp->server);
}