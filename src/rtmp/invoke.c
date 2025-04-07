#include "invoke.h"
#include <schedule/amf0.h>
#include "reply.h"

int rtmp_read_onconnect(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_oncreate_stream(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_onplay(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_onget_stream_length(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_ondelete_stream(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_onreceive_audio(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_onreceive_video(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_onpublish(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_onseek(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_onpause(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_fcpublish(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_onfcpublish(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_onfcunpublish(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_onfcsubscribe(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_onfcunsubscribe(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
int rtmp_read_releaseStream(RtmpSession *session, bs_t *b){return NET_SUCCESS; }

typedef struct
{
    const char *command;
    int (*function)(RtmpSession *session, bs_t *b);
    int (*reply)(RtmpSession *session, int code, double transactionId);
} handleInvokeCommand;

handleInvokeCommand gHandleCommand[] = {
    {"connect", rtmp_read_onconnect, rtmp_reply_connect},
    {"createStream", rtmp_read_oncreate_stream, rtmp_reply_result},
    {"getStreamLength", rtmp_read_onget_stream_length, NULL},
    {"play", rtmp_read_onplay, rtmp_reply_onplay},
    {"deleteStream", rtmp_read_ondelete_stream, NULL},
    {"receiveAudio", rtmp_read_onreceive_audio, NULL},
    {"receiveVideo", rtmp_read_onreceive_video, NULL},
    {"publish", rtmp_read_onpublish, rtmp_reply_onstatus},
    {"seek", rtmp_read_onseek, NULL},
    {"pause", rtmp_read_onpause, NULL},

    {"FCPublish", rtmp_read_fcpublish, NULL},
    {"FCUnpublish", rtmp_read_onfcunpublish, NULL},
    {"FCSubscribe", rtmp_read_onfcsubscribe, NULL},
    {"FCUnsubscribe", rtmp_read_onfcunsubscribe, NULL},
    {"releaseStream", rtmp_read_releaseStream, NULL},
    {"onFCPublish", rtmp_read_onfcpublish, rtmp_reply_result},
    {"onstatus", rtmp_read_onfcpublish, rtmp_reply_result},
};

int handleInvokeEvent(RtmpSession *session, bs_t *b)
{
    double transactionId = -1;
    uint8_t command[256] = {0};

    amf_object_item items[2];
    AMF_OBJECT_ITEM_VALUE(items[0], AMF_OBJECT, "command", command, sizeof(command));
    AMF_OBJECT_ITEM_VALUE(items[1], AMF_STRING, "transaction", &transactionId, sizeof(double));

    amf_read_item(b, items, sizeof(items) / sizeof(items[0]));

    LOG("command handle %s", command);

    for (int i = 0; i < sizeof(gHandleCommand) / sizeof(gHandleCommand[0]); i++)
    {
        if (!strncmp(gHandleCommand[i].command, command, strlen(command)))
        {
            LOG("%s", command);
            // int code = g_command_handle[i].function(b, rtmp);
            // if (g_command_handle[i].reply)
            //     g_command_handle[i].reply(rtmp, code, transactionId);
        }
    }
}