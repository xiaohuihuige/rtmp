#include "handshake.h"
#include "type.h"

void createRtmpHandShake(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    if (session->state == RTMP_HANDSHAKE_UNINIT) {
        session->state = RTMP_HANDSHAKE_0;
        Buffer * buffer = MALLOC(Buffer, sizeof(Buffer) + 2 * RTMP_HANDSHAKE_SIZE + 1);
        if (!buffer) {
            
        }
    }


    if (session->state == RTMP_HANDSHAKE_0) {
        session->state = RTMP_HANDSHAKE_1;
    }
}
