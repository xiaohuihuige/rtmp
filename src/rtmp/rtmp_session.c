#include "rtmp_session.h"

RtmpSession *createRtmpSession(Seesion *conn)
{
    RtmpSession *session = CALLOC(1, RtmpSession);
    if (!session)   
        return NULL;
        
    session->conn = conn;

    return session;
}

void destroyRtmpSession(RtmpSession *session)
{

}

void recvRtmpSession(RtmpSession *session, Buffer *buffer)
{
    LOG("recv size:%d, %p, %s", buffer->length, session, buffer->data);

    char *message = "Hello, server!";
    send(session->fd, message, strlen(message), 0);
}