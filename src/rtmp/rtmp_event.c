#include "rtmp_event.h"
#include "messages.h"
#include "rtmp_invoke.h"

static int _handleRtmpEvent(RtmpSession *session, bs_t *b)
{
    assert(session || b);
    
    int event_type = bs_read_u(b, 16);
    if (event_type == SetBufferLength)
    {    
        int stream_id    = bs_read_u(b, 32);
        int cache_lenth  = bs_read_u(b, 32);
        LOG("event_type %d,%d,%d", event_type, stream_id, cache_lenth);
    }
    //rtmp_server_stream_begin(rtmp_ptr rtmp, uint32_t streamId);
    return NET_SUCCESS;
}

static int _handleAcknowledgement(RtmpSession *session, bs_t *b)
{
    assert(session || b);

    int window_size  = bs_read_u(b, 32);

    LOG("window size %d", window_size);

    return NET_SUCCESS;
}

static int _handleSetChunkSize(RtmpSession *session, bs_t *b)
{
    assert(session || b);

    int chunk_size = bs_read_u(b, 32);

    LOG("chunk_size %d", chunk_size);

    return NET_SUCCESS;
}

static int _handleSetPeerBandwidth(RtmpSession *session, bs_t *b)
{
    assert(session || b);

    int window     = bs_read_u(b, 32);
    int limit_type = bs_read_u(b, 8);
    ERR("window %d, limit type %d", window, limit_type);
    return NET_SUCCESS;
}

static int _handleEvent(RtmpSession *session, RtmpPacket *packet)
{
    assert(packet || session);

    bs_t *b = bs_new(packet->buffer->data, packet->buffer->length);
    if (!b) 
        return NET_FAIL;

    printfMessage("revc client message:", packet->header.type_id);

    switch (packet->header.type_id)
    {
        case RTMP_TYPE_FLEX_MESSAGE:

        case RTMP_TYPE_INVOKE:
            handleInvokeEvent(session, b, &packet->header);
            break;
        case RTMP_TYPE_VIDEO:

        case RTMP_TYPE_AUDIO:

        case RTMP_TYPE_EVENT:
            _handleRtmpEvent(session, b);
            break;
        case RTMP_TYPE_SET_CHUNK_SIZE:
            _handleSetChunkSize(session, b);
            break;
        case RTMP_TYPE_ABORT:
        case RTMP_TYPE_ACKNOWLEDGEMENT:
        case RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE:
            _handleAcknowledgement(session, b);
            break;
        case RTMP_TYPE_SET_PEER_BANDWIDTH:
            _handleSetPeerBandwidth(session, b);
            break;
        case RTMP_TYPE_DATA:
            //rtmp_recv_set_onMetaData(session, b);
            break;
        case RTMP_TYPE_FLEX_STREAM:

        case RTMP_TYPE_SHARED_OBJECT:
        case RTMP_TYPE_FLEX_OBJECT:

        case RTMP_TYPE_METADATA:

        default:
            break;
    }

    FREE(b);
    FREE(packet->buffer);
    FREE(packet);
    
    return NET_SUCCESS;
}

int handleRtmpEvent(RtmpSession *session, RtmpPacket *packet)
{
    if (!session || !packet)
        return NET_FAIL;

    return _handleEvent(session, packet);
}