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
    return sendPeerBandwidth(session, RTMP_WINDOW_SIZE, RTMP_BANDWIDTH_LIMIT_DYNAMIC);  
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