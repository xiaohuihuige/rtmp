#include "rtmp_event.h"
#include "messages.h"
#include "invoke.h"

static int _handleEvent(RtmpSession *session, RtmpPacket *packet)
{
    assert(packet || session);

    LOG("---------");

    bs_t *b = bs_new(packet->buffer->data, packet->buffer->length);
    if (!b) 
        return NET_FAIL;
    
    printfMessage("revc client message:", packet->header.type_id);

    switch (packet->header.type_id)
    {
        case RTMP_TYPE_FLEX_MESSAGE:

        case RTMP_TYPE_INVOKE:
            handleInvokeEvent(session, b);
            break;
        case RTMP_TYPE_VIDEO:

        case RTMP_TYPE_AUDIO:

        case RTMP_TYPE_EVENT:
            break;
        case RTMP_TYPE_SET_CHUNK_SIZE:
            break;
        case RTMP_TYPE_ABORT:
        case RTMP_TYPE_ACKNOWLEDGEMENT:
        case RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE:
            break;
        case RTMP_TYPE_SET_PEER_BANDWIDTH:
            break;
        case RTMP_TYPE_DATA:
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
    return _handleEvent(session, packet);
}