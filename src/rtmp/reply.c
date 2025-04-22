#include "reply.h"

#define RTMP_WINDOW_SIZE 5000000
enum
{
    RTMP_BANDWIDTH_LIMIT_HARD = 0,
    RTMP_BANDWIDTH_LIMIT_SOFT = 1,
    RTMP_BANDWIDTH_LIMIT_DYNAMIC = 2,
};
#define RTMP_OUTPUT_CHUNK_SIZE 4096

int rtmp_reply_connect(RtmpSession *session, int code, double transactionId)
{
    Buffer *buffer = createBuffer(300);
    if (!buffer)
        return NET_FAIL;

    sendPeerBandwidth(session, buffer, RTMP_BANDWIDTH_LIMIT_DYNAMIC);  

    sendAcknowledgement(session, buffer, RTMP_WINDOW_SIZE);

    sendChunkSize(session, buffer, RTMP_OUTPUT_CHUNK_SIZE);

    sendConnectResult(session, buffer, transactionId);

    FREE(buffer);
}

int rtmp_reply_result(RtmpSession *session, int code, double transactionId)
{
return NET_SUCCESS; }
int rtmp_reply_onplay(RtmpSession *session, int code, double transactionId)
{
return NET_SUCCESS; }
int rtmp_reply_onstatus(RtmpSession *session, int code, double transactionId)
{
return NET_SUCCESS; }