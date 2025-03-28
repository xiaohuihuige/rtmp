#include "handshake.h"
#include "type.h"

void handshakeRtmp(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    if (session->state == RTMP_HANDSHAKE_UNINIT) {
        session->state = RTMP_HANDSHAKE_0;
    }


    if (session->state == RTMP_HANDSHAKE_0) {
        session->state = RTMP_HANDSHAKE_1;
    }
}
