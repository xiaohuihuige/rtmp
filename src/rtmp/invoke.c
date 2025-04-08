#include "invoke.h"
#include <schedule/amf0.h>
#include "reply.h"

int rtmp_read_onconnect(RtmpSession *session, bs_t *b)
{
    if (b == NULL ||rtmp == NULL)
        return NET_FAIL;

    //retrieve(rtmp->buffer, readable_bytes(rtmp->buffer));
    
    rtmp_connect *conn = &rtmp->conn;

	amf_object_item items[1];
	amf_object_item commands[4];

	conn->encoding = (double)RTMP_ENCODING_AMF_0;

    AMF_OBJECT_ITEM_VALUE(commands[0], AMF_STRING, "app", conn->app, sizeof(conn->app));
	AMF_OBJECT_ITEM_VALUE(commands[1], AMF_STRING, "type", conn->type, sizeof(conn->type));
	AMF_OBJECT_ITEM_VALUE(commands[2], AMF_STRING, "flashVer", conn->flashver, sizeof(conn->flashver));
    AMF_OBJECT_ITEM_VALUE(commands[3], AMF_STRING, "tcUrl", conn->tcUrl, sizeof(conn->tcUrl));
    // AMF_OBJECT_ITEM_VALUE(commands[3], AMF_BOOLEAN, "fpad", &conn->fpad, 1);
    // AMF_OBJECT_ITEM_VALUE(commands[4], AMF_NUMBER, "capabilities", &conn->capabilities, 8);
	// AMF_OBJECT_ITEM_VALUE(commands[5], AMF_NUMBER, "audioCodecs", &conn->audioCodecs, 8);
 	// AMF_OBJECT_ITEM_VALUE(commands[6], AMF_NUMBER, "videoCodecs", &conn->videoCodecs, 8);
 	// AMF_OBJECT_ITEM_VALUE(commands[7], AMF_NUMBER, "videoFunction", &conn->videoFunction, 8);
 	// AMF_OBJECT_ITEM_VALUE(commands[8], AMF_NUMBER, "objectEncoding", &conn->encoding, 8);
	AMF_OBJECT_ITEM_VALUE(items[0],    AMF_OBJECT, "command", commands, sizeof(commands) / sizeof(commands[0]));
    
    return amf_read_object_item(b, bs_read_u8(b), items);
    return NET_SUCCESS; 
}



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
        if (!strncmp(gHandleCommand[i].command, (char *)command, strlen((char *)command)))
        {
            int code = gHandleCommand[i].function(session, b);
            if (gHandleCommand[i].reply)
                gHandleCommand[i].reply(session, code, transactionId);
        }
    }
    return NET_SUCCESS;
}