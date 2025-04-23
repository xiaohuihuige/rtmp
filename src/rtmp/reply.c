#include "reply.h"
#include "control_message.h"

#define RTMP_WINDOW_SIZE 5000000
enum
{
    RTMP_BANDWIDTH_LIMIT_HARD = 0,
    RTMP_BANDWIDTH_LIMIT_SOFT = 1,
    RTMP_BANDWIDTH_LIMIT_DYNAMIC = 2,
};
#define RTMP_OUTPUT_CHUNK_SIZE 4096

int rtmp_reply_connect(RtmpSession *session, HeaderChunk *header, int code, double transactionId)
{
    Buffer *buffer = createBuffer(300);
    if (!buffer)
        return NET_FAIL;

    sendPeerBandwidth(session, buffer, RTMP_BANDWIDTH_LIMIT_DYNAMIC, RTMP_BANDWIDTH_LIMIT_DYNAMIC);  

    sendAcknowledgement(session, buffer, RTMP_WINDOW_SIZE);

    sendChunkSize(session, buffer, RTMP_OUTPUT_CHUNK_SIZE);

    sendConnectResult(session, buffer, transactionId);

    FREE(buffer);

    return NET_SUCCESS; 
}

int rtmp_reply_result(RtmpSession *session, HeaderChunk *header, int code, double transactionId)
{
    Buffer *buffer = createBuffer(300);
    if (!buffer)
        return NET_FAIL;

    sendCreateStreamResult(session, buffer, transactionId, header->stream_id);

    FREE(buffer);

    return NET_SUCCESS; 
}

int rtmp_reply_onplay(RtmpSession *session, HeaderChunk *header, int code, double transactionId)
{
    Buffer *buffer = createBuffer(500);
    if (!buffer)
        return NET_FAIL;

    sendSetStreamBegin(session, buffer, header->stream_id);

    sendOnMetaData(session, buffer);

    sendSampleAccess(session, buffer, header->stream_id);

    sendOnstatus(session, buffer, transactionId, session->config.app);

    return NET_SUCCESS; 
}

int rtmp_reply_onstatus(RtmpSession *session, HeaderChunk *header, int code, double transactionId)
{
    return NET_SUCCESS; 
}