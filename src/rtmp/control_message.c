#include "control_message.h"
#include "rtmp_session.h"
#include "type.h"
#include "amf0.h"
#include "send_chunk.h"

static void _buildPeerBandwidth(bs_t *b, uint32_t window_size, uint8_t limit_type);
static void _buildSetChunkSize(bs_t *b, int chunk_size);
void _buildSetAbort(bs_t *b, uint32_t chunk_streamid);
void _buildSetAcknowledgement(bs_t *b, uint32_t sequence_number);
static void _buildWindowAcknowledgementSize(bs_t *b, uint32_t window_size);
static void _buildSetStreamBegin(bs_t *b, uint32_t streamId);
void _buildSetStreamEof(bs_t *b, uint32_t streamId);
void _buildStreamDry(bs_t *b, uint32_t streamId);
void _buildSetBufferLength(bs_t *b, uint32_t streamId, uint32_t ms);
void _buildSetStreamIsRecord(bs_t *b, uint32_t streamId);
void _buildSetPing(bs_t *b, uint32_t timstamp);
void _buildSetPong(bs_t *b, uint32_t timstamp);
static void _buildConnectResult(bs_t *b, double transactionId, const char* fmsver, double capabilities, const char* code, const char* level, const char* description, double encoding);
static void _buildCreateStreanResult(bs_t *b, double transactionId, double stream_id);
static void _buildWriteOnstatus(bs_t *b,  double transactionId, const char* level, const char* code, const char* description);
static void _buildSampleAccess(bs_t *b,  double stream_id, const char *sample_access);
void _buildStreamIsRecord(bs_t *b, uint32_t streamId);
static void _buildOnMetaData(bs_t *b, VideoMedia *video, AudioMedia *audio);


static void _buildPeerBandwidth(bs_t *b, uint32_t window_size, uint8_t limit_type)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 5,
        .type_id = RTMP_TYPE_SET_PEER_BANDWIDTH,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 32, window_size);
    bs_write_u8(b, limit_type); 
}

static void _buildSetChunkSize(bs_t *b, int chunk_size)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 4,
        .type_id = RTMP_TYPE_SET_CHUNK_SIZE,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 32, chunk_size & 0x7FFFFFFF);
}

void _buildSetAbort(bs_t *b, uint32_t chunk_streamid)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 4,
        .type_id = RTMP_TYPE_ABORT,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 32, chunk_streamid);
}

void _buildSetAcknowledgement(bs_t *b, uint32_t sequence_number)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 4,
        .type_id = RTMP_TYPE_ACKNOWLEDGEMENT,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 32, sequence_number);
}

static void _buildWindowAcknowledgementSize(bs_t *b, uint32_t window_size)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 4,
        .type_id = RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 32, window_size);
}

static void _buildSetStreamBegin(bs_t *b, uint32_t streamId)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 6,
        .type_id = RTMP_TYPE_EVENT,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 16, RTMP_EVENT_STREAM_BEGIN);
    bs_write_u(b, 32, streamId);
}


void _buildSetStreamEof(bs_t *b, uint32_t streamId)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 6,
        .type_id = RTMP_TYPE_EVENT,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 16, RTMP_EVENT_STREAM_EOF);
    bs_write_u(b, 32, streamId);
}

void _buildStreamDry(bs_t *b, uint32_t streamId)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 6,
        .type_id = RTMP_TYPE_EVENT,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 16, RTMP_EVENT_STREAM_DRY);
    bs_write_u(b, 32, streamId);
}   

void _buildSetBufferLength(bs_t *b, uint32_t streamId, uint32_t ms)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 10,
        .type_id = RTMP_TYPE_EVENT,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 16, RTMP_EVENT_SET_BUFFER_LENGTH);
    bs_write_u(b, 32, streamId);
    bs_write_u(b, 32, ms);
}

void _buildSetStreamIsRecord(bs_t *b, uint32_t streamId)
{
	HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 6,
        .type_id = RTMP_TYPE_EVENT,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 16, RTMP_EVENT_STREAM_IS_RECORD);
    bs_write_u(b, 32, streamId);
}

void _buildSetPing(bs_t *b, uint32_t timstamp)
{
	HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 6,
        .type_id = RTMP_TYPE_EVENT,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 16, RTMP_EVENT_PING);
    bs_write_u(b, 32, timstamp);
}

void _buildSetPong(bs_t *b, uint32_t timstamp)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 6,
        .type_id = RTMP_TYPE_EVENT,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 16, RTMP_EVENT_PONG);
    bs_write_u(b, 32, timstamp);
}

static void _buildConnectResult(bs_t *b, 
            double transactionId, const char* fmsver, 
            double capabilities, const char* code, 
            const char* level, const char* description, double encoding)
{

    int length = AMF_STRING_LENGTH("_result") 
                + AMF_DOUBLE_LENGTH 
                + AMF_OBJECT_LENGTH
                + AMF_NAMESTRING_LENGTH("fmsVer", fmsver)
                + AMF_NAMEDOUBLE_LENGTH("capabilities")
                + AMF_NAMEDOUBLE_LENGTH("mode")
                + AMF_OBJECT_END_LENGTH
                + AMF_OBJECT_LENGTH
                + AMF_NAMESTRING_LENGTH("level", level)
                + AMF_NAMESTRING_LENGTH("code", code)
                + AMF_NAMESTRING_LENGTH("description", description)
                + AMF_NAMEDOUBLE_LENGTH("objectEncoding")
                + AMF_OBJECT_END_LENGTH;

    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_INVOKE,
        .timestamp = 0,
        .length = length, //205
        .type_id = RTMP_TYPE_INVOKE,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    amf_write_string(b, "_result", strlen("_result"));
	amf_write_double(b, transactionId);

	amf_write_object(b);
	amf_write_NamedString(b,  "fmsVer", strlen("fmsVer"), fmsver, strlen(fmsver));
	amf_write_NamedDouble(b,  "capabilities", strlen("capabilities"), capabilities);
	amf_write_NamedDouble(b,  "mode", strlen("mode"), 1);
	amf_write_objectEnd(b);
 
	amf_write_object(b);
	amf_write_NamedString(b, "level", strlen("level"), level, strlen(level));
	amf_write_NamedString(b, "code", strlen("code"), code, strlen(code));
	amf_write_NamedString(b, "description", strlen("description"), description, strlen(description));
	amf_write_NamedDouble(b, "objectEncoding", strlen("objectEncoding"), encoding);
	amf_write_objectEnd(b);
}

static void _buildCreateStreanResult(bs_t *b, double transactionId, double stream_id)
{

    int length = AMF_STRING_LENGTH("_result")  + AMF_DOUBLE_LENGTH + AMF_NULL_LENGTH + AMF_DOUBLE_LENGTH;

    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_INVOKE,
        .timestamp = 0,
        .length = length,
        .type_id = RTMP_TYPE_INVOKE,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    amf_write_string(b, "_result", strlen("_result"));
	amf_write_double(b, transactionId);
	amf_write_null(b);
	amf_write_double(b, stream_id);
}

static void _buildWriteOnstatus(bs_t *b,  double transactionId, const char* level, const char* code, const char* description)
{
    int length = AMF_STRING_LENGTH("onStatus")  
                + AMF_DOUBLE_LENGTH 
                + AMF_NULL_LENGTH 
                + AMF_OBJECT_LENGTH 
                + AMF_NAMESTRING_LENGTH("level", level)
                + AMF_NAMESTRING_LENGTH("code", code)
                + AMF_NAMESTRING_LENGTH("description", description) 
                + AMF_OBJECT_END_LENGTH;

    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_INVOKE,
        .timestamp = 0,
        .length = length,
        .type_id = RTMP_TYPE_INVOKE,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    amf_write_string(b, "onStatus", strlen("onStatus"));
	amf_write_double(b, transactionId);
	amf_write_null(b);

	amf_write_object(b);
	amf_write_NamedString(b, "level", strlen("level"), level, strlen(level));
	amf_write_NamedString(b, "code", strlen("code"),  code, strlen(code));
	amf_write_NamedString(b, "description", strlen("description"), description, strlen(description));
	amf_write_objectEnd(b);
}


static void _buildSampleAccess(bs_t *b,  double stream_id, const char *sample_access)
{
    int length = AMF_STRING_LENGTH(sample_access)  + AMG_BOOLEAN_LENGTH + AMG_BOOLEAN_LENGTH;
        
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_DATA,
        .timestamp = 0,
        .length = length,
        .type_id = RTMP_TYPE_DATA,
        .stream_id = stream_id,
    };

    writeChunkHeader(b, &header);

    amf_write_string(b, sample_access, strlen(sample_access));
	amf_write_boolean(b, 1);
	amf_write_boolean(b, 1);
}

static void _buildOnMetaData(bs_t *b, VideoMedia *video, AudioMedia *audio)
{
    int length = AMF_STRING_LENGTH("onMetaData")  
                + AMF_OBJECT_LENGTH
                + AMF_NAMESTRING_LENGTH("Server", "nginx-rtmp-module")
                + AMF_NAMEDOUBLE_LENGTH("width")
                + AMF_NAMEDOUBLE_LENGTH("height")
                + AMF_NAMEDOUBLE_LENGTH("displayWidth")
                + AMF_NAMEDOUBLE_LENGTH("displayHeight")
                + AMF_NAMEDOUBLE_LENGTH("duration")
                + AMF_NAMEDOUBLE_LENGTH("framerate")
                + AMF_NAMEDOUBLE_LENGTH("fps")
                + AMF_NAMEDOUBLE_LENGTH("videodatarate")
                + AMF_NAMEDOUBLE_LENGTH("videocodecid")
                + AMF_NAMEDOUBLE_LENGTH("audiodatarate") 
                + AMF_NAMEDOUBLE_LENGTH("audiocodecid")
                + AMF_NAMESTRING_LENGTH("profile", "") 
                + AMF_NAMESTRING_LENGTH("level", "") 
                + AMF_NAMEDOUBLE_LENGTH("audiosamplerate")
                + AMF_NAMEDOUBLE_LENGTH("audiosamplesize")
                + AMF_NAMEBOOLEAN_LENGTH("stereo")
                + AMF_OBJECT_END_LENGTH;

    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_VIDEO,
        .timestamp = 0,
        .length = length,
        .type_id = RTMP_TYPE_DATA,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    amf_write_string(b, "onMetaData", strlen("onMetaData"));

	amf_write_object(b);
	amf_write_NamedString(b, "Server", strlen("Server"), "nginx-rtmp-module", strlen("nginx-rtmp-module"));
	amf_write_NamedDouble(b,  "width",   strlen("width"),  video->width);
	amf_write_NamedDouble(b,  "height",  strlen("height"),  video->height);
	amf_write_NamedDouble(b,  "displayWidth",   strlen("displayWidth"),  video->width);
	amf_write_NamedDouble(b,  "displayHeight",   strlen("displayHeight"),  video->height);
	amf_write_NamedDouble(b,  "duration",   strlen("duration"),  DURATION);
	amf_write_NamedDouble(b,  "framerate",   strlen("framerate"),  video->fps);
	amf_write_NamedDouble(b,  "fps",         strlen("fps"),  video->fps);
	amf_write_NamedDouble(b,  "videodatarate",   strlen("videodatarate"),  VIDEODATARATE);
	amf_write_NamedDouble(b,  "videocodecid",   strlen("videocodecid"),  VIDEOCODECID);
	amf_write_NamedDouble(b,  "audiodatarate",   strlen("audiodatarate"),  audio->audiodatarate);
	amf_write_NamedDouble(b,  "audiocodecid",   strlen("audiocodecid"),  audio->audiocodecid);

    amf_write_NamedDouble(b, "audiosamplerate", strlen("audiosamplerate"), audio->audiosamplerate);
    amf_write_NamedDouble(b, "audiosamplesize", strlen("audiosamplesize"), audio->audiosamplesize);
    amf_write_NamedBoolean(b, "stereo", strlen("stereo"), 1);

    amf_write_NamedString(b, "profile", strlen("profile"), "", 0);
	amf_write_NamedString(b, "level", strlen("level"),  "", 0);
	amf_write_objectEnd(b);
}

void _buildStreamIsRecord(bs_t *b, uint32_t streamId)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 6,
        .type_id = RTMP_TYPE_EVENT,
        .stream_id = 0,
    };

    writeChunkHeader(b, &header);

    bs_write_u(b, 16, RTMP_EVENT_STREAM_IS_RECORD);
    bs_write_u(b, 32, streamId);
}


int sendPeerBandwidth(RtmpSession *session, Buffer *buffer, uint32_t window_size, uint8_t limit_type)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    _buildPeerBandwidth(b,  window_size, limit_type);

    sendToClient(session, buffer->data, bs_pos(b));

    //printfChar(buffer->data, bs_pos(b));

    FREE(b);

    return NET_SUCCESS;
}


int sendAcknowledgement(RtmpSession *session, Buffer *buffer, uint32_t acknowledgement)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    _buildWindowAcknowledgementSize(b, acknowledgement);

    sendToClient(session, buffer->data, bs_pos(b));

    //printfChar(buffer->data, bs_pos(b));

    FREE(b);

    return NET_SUCCESS;
}

int sendChunkSize(RtmpSession *session, Buffer *buffer, int chunk_size)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    _buildSetChunkSize(b,  chunk_size);

    sendToClient(session, buffer->data, bs_pos(b));

    //printfChar(buffer->data, bs_pos(b));

    FREE(b);

    return NET_SUCCESS;
}

int sendConnectResult(RtmpSession *session, Buffer *buffer, double transactionId)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    _buildConnectResult(b, transactionId , RTMP_FMSVER, 
                                    RTMP_CAPABILITIES, "NetConnection.Connect.Success", 
                                    RTMP_LEVEL_STATUS, "Connection succeeded.", 0.0);

    sendToClient(session, buffer->data, bs_pos(b));

    //printfChar(buffer->data, bs_pos(b));

    FREE(b);

    return NET_SUCCESS;
}


int sendCreateStreamResult(RtmpSession *session, Buffer *buffer, double transactionId, uint32_t stream_id)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    _buildCreateStreanResult(b, transactionId, stream_id);

    sendToClient(session, buffer->data, bs_pos(b));

    FREE(b);

    return NET_SUCCESS;
} 

int sendSetStreamBegin(RtmpSession *session, Buffer *buffer, uint32_t stream_id)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    _buildSetStreamBegin(b, stream_id);

    sendToClient(session, buffer->data, bs_pos(b));

    FREE(b);

    return NET_SUCCESS;
}

int sendOnstatus(RtmpSession *session, Buffer *buffer, double transactionId,  const char *app)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    char message[140] = {0};
    sprintf(message, "start %s", app);
    _buildWriteOnstatus(b, transactionId, RTMP_LEVEL_STATUS, "NetStream.Play.Start", message);

    sendToClient(session, buffer->data, bs_pos(b));

    FREE(b);

    return NET_SUCCESS;

}

int sendSampleAccess(RtmpSession *session, Buffer *buffer,  uint32_t stream_id)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    _buildSampleAccess(b, stream_id, "|RtmpSampleAccess");

    sendToClient(session, buffer->data, bs_pos(b));

    FREE(b);

    return NET_SUCCESS;
}

int sendOnMetaData(RtmpSession *session, Buffer *buffer)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    LOG("media info width %d, height %d, fps %d, profile %d, level %d",
        session->media->video->width, 
        session->media->video->height, 
        session->media->video->fps, 
        session->media->video->profile_idc, 
        session->media->video->level_idc);

    _buildOnMetaData(b, session->media->video, session->media->audio);

    sendToClient(session, buffer->data, bs_pos(b));

    FREE(b);

    return NET_SUCCESS;
}
