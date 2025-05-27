#include "rtmp_invoke.h"
#include <schedule/amf0.h>
#include "rtmp_reply.h"

typedef struct
{
    const char *command;
    int (*function)(RtmpSession *session, bs_t *b);
    int (*reply)(RtmpSession *session, HeaderChunk *header, int code, double transactionId);
} handleInvokeCommand;

static int _rtmpReadOnconnect(RtmpSession *session, bs_t *b)
{
    assert(session || b);

	amf_object_item items;
	amf_object_item commands[10];

	session->config.encoding = (double)RTMP_ENCODING_AMF_0;
    
    AMF_OBJECT_ITEM_VALUE(commands[0], AMF_STRING, "app", session->config.app, sizeof(session->config.app));
	AMF_OBJECT_ITEM_VALUE(commands[1], AMF_STRING, "type", session->config.type, sizeof(session->config.type));
	AMF_OBJECT_ITEM_VALUE(commands[2], AMF_STRING, "flashVer", session->config.flashver, sizeof(session->config.flashver));
    AMF_OBJECT_ITEM_VALUE(commands[3], AMF_STRING, "tcUrl", session->config.tcUrl, sizeof(session->config.tcUrl));
    AMF_OBJECT_ITEM_VALUE(commands[4], AMF_BOOLEAN, "fpad", &session->config.fpad, 1);
    AMF_OBJECT_ITEM_VALUE(commands[5], AMF_NUMBER, "capabilities", &session->config.capabilities, 8);
	AMF_OBJECT_ITEM_VALUE(commands[6], AMF_NUMBER, "audioCodecs", &session->config.audioCodecs, 8);
 	AMF_OBJECT_ITEM_VALUE(commands[7], AMF_NUMBER, "videoCodecs", &session->config.videoCodecs, 8);
 	AMF_OBJECT_ITEM_VALUE(commands[8], AMF_NUMBER, "videoFunction", &session->config.videoFunction, 8);
 	AMF_OBJECT_ITEM_VALUE(commands[9], AMF_NUMBER, "objectEncoding", &session->config.encoding, 8);
	AMF_OBJECT_ITEM_VALUE(items,       AMF_OBJECT, "command", commands, sizeof(commands) / sizeof(commands[0]));
    
    amf_read_object_item(b, &items);

    LOG("app: %s, flashver: %s, type: %s, swfUrl: %s, tcUrl: %s, fpad: %d, capabilities: %f, audioCodecs: %f, videoCodecs: %f, videoFunction: %f, encoding: %f, pageUrl: %s",
        session->config.app, session->config.flashver,
        session->config.type, session->config.swfUrl,
        session->config.tcUrl, session->config.fpad,
        session->config.capabilities, session->config.audioCodecs,
        session->config.videoCodecs, session->config.videoFunction,
        session->config.encoding, session->config.pageUrl);

    return NET_SUCCESS;
}



int _rtmpReadOncreateStream(RtmpSession *session, bs_t *b){return NET_SUCCESS; }
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


handleInvokeCommand gHandleCommand[] = {
    {"connect",             _rtmpReadOnconnect,             rtmpSendConnect},
    {"createStream",        _rtmpReadOncreateStream,        rtmpSendResult},
    {"getStreamLength",     rtmp_read_onget_stream_length,  NULL},
    {"play",                rtmp_read_onplay,               rtmpSendOnplay},
    {"deleteStream",        rtmp_read_ondelete_stream,      NULL},
    {"receiveAudio",        rtmp_read_onreceive_audio,      NULL},
    {"receiveVideo",        rtmp_read_onreceive_video,      NULL},
    {"publish",             rtmp_read_onpublish,            rtmpSendOnstatus},
    {"seek",                rtmp_read_onseek,               NULL},
    {"pause",               rtmp_read_onpause,              NULL},

    {"FCPublish",           rtmp_read_fcpublish,            NULL},
    {"FCUnpublish",         rtmp_read_onfcunpublish,        NULL},
    {"FCSubscribe",         rtmp_read_onfcsubscribe,        NULL},
    {"FCUnsubscribe",       rtmp_read_onfcunsubscribe,      NULL},
    {"releaseStream",       rtmp_read_releaseStream,        NULL},
    {"onFCPublish",         rtmp_read_onfcpublish,          rtmpSendResult},
    {"onstatus",            rtmp_read_onfcpublish,          rtmpSendResult},
};

int handleInvokeEvent(RtmpSession *session, bs_t *b, HeaderChunk *header)
{
    if (!session || !b || !header)
        return NET_FAIL;

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
                gHandleCommand[i].reply(session, header, code, transactionId);
            break;
        }
    }
    return NET_SUCCESS;
}