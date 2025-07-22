#include "handshake.h"
#include "type.h"
#include "send_chunk.h"

static void _buildHandShakeRandom(bs_t *b, int length)
{
    assert(b);

    srand(time(NULL));
    while (length--) bs_write_u8(b, rand());
}

int sendHandShakeS0S1S2(RtmpSession *session, Buffer *buffer)
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
    bs_write_bytes(b, buffer->data + 1, RTMP_HANDSHAKE_SIZE - 8);

    sendToClient(session, send_buffer->data, send_buffer->length);

    FREE(b);
    FREE(send_buffer);

    return NET_SUCCESS;
}
