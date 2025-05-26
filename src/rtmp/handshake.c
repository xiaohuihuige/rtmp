#include "handshake.h"
#include "type.h"
#include "send_chunk.h"

static void _buildHandShakeRandom(bs_t *b, int length)
{
    assert(b);

    srand(time(NULL));
    while (length--) bs_write_u8(b, rand());
}

static int _sendHandShakeS0S1S2(RtmpSession *session, Buffer *buffer)
{
    assert(session || buffer);

    Buffer *send_buffer = createBuffer(2 * RTMP_HANDSHAKE_SIZE + 1);
    if (!send_buffer)
        return NET_FAIL;

    bs_t *b = bs_new(send_buffer->data, send_buffer->length);
    if (!b) {
        FREE(send_buffer);
        return NET_FAIL;
    }

    bs_write_u8(b, RTMP_VERSION);

    bs_write_u(b, 32, time(NULL));
    bs_write_u(b, 32, 0);
    _buildHandShakeRandom(b, RTMP_HANDSHAKE_SIZE - 8);

    bs_write_u(b, 32, time(NULL));
    bs_write_u(b, 32, 0);
    _buildHandShakeRandom(b, RTMP_HANDSHAKE_SIZE - 8);

    sendToClient(session, send_buffer->data, send_buffer->length);

    FREE(b);
    FREE(send_buffer);

    return NET_SUCCESS;
}


void createRtmpHandShake(RtmpSession *session, Buffer *buffer)
{
    if (!session || !buffer)
        return;

    if (session->state == RTMP_HANDSHAKE_UNINIT) {
        session->state = RTMP_HANDSHAKE_0;
        _sendHandShakeS0S1S2(session, buffer);
        return;
    }

    if (session->state == RTMP_HANDSHAKE_0) {
        session->state = RTMP_HANDSHAKE_1;
    }
    return;
}
