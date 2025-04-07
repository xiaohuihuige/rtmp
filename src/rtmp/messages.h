#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "type.h"
#include <schedule/net-common.h>
#include "invoke.h"
#include "reply.h"
#include "rtmp_session.h"

typedef struct 
{
    int code;
    char error[126];
} stateMessage;

typedef struct
{
    const char *command;
    int (*function)(RtmpSession *session, bs_t *b);
    int (*reply)(RtmpSession *session, int code, double transactionId);
} handleInvokeCommand;

stateMessage state[] = {
    {200, "result"},
    {RTMP_TYPE_SET_CHUNK_SIZE, "RTMP_TYPE_SET_CHUNK_SIZE"},
    {RTMP_TYPE_ABORT, "RTMP_TYPE_ABORT"},
    {RTMP_TYPE_ACKNOWLEDGEMENT, "RTMP_TYPE_ACKNOWLEDGEMENT"},
    {RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE, "RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE"},
    {RTMP_TYPE_SET_PEER_BANDWIDTH, "RTMP_TYPE_SET_PEER_BANDWIDTH"},
    {RTMP_TYPE_EVENT, "RTMP_TYPE_EVENT"},
    {RTMP_TYPE_AUDIO, "RTMP_TYPE_AUDIO"},
    {RTMP_TYPE_VIDEO, "RTMP_TYPE_VIDEO"},
    {RTMP_TYPE_FLEX_STREAM, "RTMP_TYPE_FLEX_STREAM"},
    {RTMP_TYPE_DATA, "RTMP_TYPE_DATA"},
    {RTMP_TYPE_FLEX_OBJECT, "RTMP_TYPE_FLEX_OBJEC"},
    {RTMP_TYPE_SHARED_OBJECT, "RTMP_TYPE_SHARED_OBJECT"},
    {RTMP_TYPE_FLEX_MESSAGE, "RTMP_TYPE_FLEX_MESSAGE"},
    {RTMP_TYPE_INVOKE, "RTMP_TYPE_INVOKE"},
    {RTMP_TYPE_METADATA, "RTMP_TYPE_METADATA"},
};

static inline void printfMessage(char *message, int code)
{
    for (int i = 0; i < sizeof(state)/sizeof(stateMessage); i++) {
        if (code == state[i].code)
            LOG("%s %s", message, state[i].error);
    }
}

handleInvokeCommand gHandleCommand[] = {
    {"connect",         rtmp_read_onconnect,            rtmp_reply_connect},
    {"createStream",    rtmp_read_oncreate_stream,      rtmp_reply_result},
    {"getStreamLength", rtmp_read_onget_stream_length,  NULL},
    {"play",            rtmp_read_onplay,               rtmp_reply_onplay},
    {"deleteStream",    rtmp_read_ondelete_stream,      NULL},
    {"receiveAudio",    rtmp_read_onreceive_audio,      NULL},
    {"receiveVideo",    rtmp_read_onreceive_video,      NULL},
    {"publish",         rtmp_read_onpublish,            rtmp_reply_onstatus},
    {"seek",            rtmp_read_onseek,               NULL},
    {"pause",           rtmp_read_onpause,              NULL},

    {"FCPublish",       rtmp_read_fcpublish,            NULL},
    {"FCUnpublish",     rtmp_read_onfcunpublish,        NULL},
    {"FCSubscribe",     rtmp_read_onfcsubscribe,        NULL},
    {"FCUnsubscribe",   rtmp_read_onfcunsubscribe,      NULL},
    {"releaseStream",   rtmp_read_releaseStream,        NULL},
    {"onFCPublish",     rtmp_read_onfcpublish,          rtmp_reply_result},
    {"onstatus",        rtmp_read_onfcpublish,          rtmp_reply_result},
};

#endif // !__MESSAGES_H__
