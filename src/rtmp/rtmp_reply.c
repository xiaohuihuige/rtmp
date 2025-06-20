#include "rtmp_reply.h"
#include "control_message.h"
#include "rtmp_media.h"
#include "rtmp_server.h"

int rtmpSendConnect(RtmpSession *session, HeaderChunk *header, int code, double transactionId)
{
    if (!session || !header)
        return NET_FAIL;

    Buffer *buffer = createBuffer(500);
    if (!buffer)
        return NET_FAIL;

    session->media = findRtmpServerMedia((RtmpServer *)session->conn->tcps->parent, session->config.app);
    if (!session->media)
    {
        ERR("find rtmp stream error");
        sendStreamError(session, buffer, transactionId, header->stream_id);
        FREE(buffer);
        return NET_FAIL;
    }

    LOG("find stream media [%p], name [%s]", session->media, session->media->app);

    sendPeerBandwidth(session, buffer, RTMP_WINDOW_SIZE, RTMP_BANDWIDTH_LIMIT_DYNAMIC);  

    sendAcknowledgement(session, buffer, RTMP_WINDOW_SIZE);

    sendChunkSize(session, buffer, RTMP_OUTPUT_CHUNK_SIZE);

    sendConnectResult(session, buffer, transactionId);

    FREE(buffer);

    return NET_SUCCESS; 
}

int rtmpSendResult(RtmpSession *session, HeaderChunk *header, int code, double transactionId)
{
    if (!session || !header)
        return NET_FAIL;

    Buffer *buffer = createBuffer(300);
    if (!buffer)
        return NET_FAIL;

    sendCreateStreamResult(session, buffer, transactionId, header->stream_id);

    FREE(buffer);

    return NET_SUCCESS; 
}

int rtmpSendOnplay(RtmpSession *session, HeaderChunk *header, int code, double transactionId)
{
    if (!session || !header)
        return NET_FAIL;

    Buffer *buffer = createBuffer(500);
    if (!buffer)
        return NET_FAIL;

    sendSetStreamBegin(session, buffer, header->stream_id);

    sendOnMetaData(session, buffer);

    sendSampleAccess(session, buffer, header->stream_id);

    sendOnstatus(session, buffer, transactionId, session->config.app);

    FREE(buffer);

    addRtmpSessionToMedia(session->media, session);

    return NET_SUCCESS; 
}

int rtmpSendOnstatus(RtmpSession *session, HeaderChunk *header, int code, double transactionId)
{
    if (!session || !header)
        return NET_FAIL;

    return NET_SUCCESS; 
}