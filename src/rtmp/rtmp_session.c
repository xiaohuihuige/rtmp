#include "rtmp_session.h"
#include "type.h"
#include "handshake.h"

RtmpSession *createRtmpSession(Seesion *conn)
{
    RtmpSession *session = CALLOC(1, RtmpSession);
    if (!session)   
        return NULL;

    session->conn  = conn;
    session->state = RTMP_HANDSHAKE_UNINIT;

    return session;
}

void destroyRtmpSession(RtmpSession *session)
{
    LOG("close");
    FREE(session);
}

static void _parseRtmpPacket(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    if (session->state == RTMP_HANDSHAKE_UNINIT || session->state == RTMP_HANDSHAKE_0)
        return createRtmpHandShake(session, buffer);
}

void recvRtmpSession(RtmpSession *session, Buffer *buffer)
{
    LOG("recv size:%d, %p, %s", buffer->length, session, buffer->data);

    //char *message = "Hello, server!";
    //send(session->conn->fd, message, strlen(message), 0);
    return _parseRtmpPacket(session, buffer);
}


