#include "control_message.h"
#include "rtmp_session.h"
#include "type.h"
#include "amf0.h"

static void _buildPeerBandwidth(bs_t *b, uint32_t window_size, uint8_t limit_type)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_PROTOCOL,
        .timestamp = 0,
        .length = 5,
        .type_id = RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE,
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

static void _buildSetAbort(bs_t *b, uint32_t chunk_streamid)
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

static void _buildSetAcknowledgement(bs_t *b, uint32_t sequence_number)
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


static void _buildSetStreamEof(bs_t *b, uint32_t streamId)
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

static void _buildStreamDry(bs_t *b, uint32_t streamId)
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

static void _buildSetBufferLength(bs_t *b, uint32_t streamId, uint32_t ms)
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

static void _buildSetStreamIsRecord(bs_t *b, uint32_t streamId)
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

static void _buildSetPing(bs_t *b, uint32_t timstamp)
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

static void  _builkSetPong(bs_t *b, uint32_t timstamp)
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
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_INVOKE,
        .timestamp = 0,
        .length = 0,
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


int sendPeerBandwidth(RtmpSession *session, Buffer *buffer, uint32_t window_size, uint8_t limit_type)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    _buildPeerBandwidth(b,  window_size, limit_type);

    send(session->conn->fd, buffer->data, bs_pos(b), 0);

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

    send(session->conn->fd, buffer->data, bs_pos(b), 0);

    //printfChar(buffer->data, bs_pos(b));

    FREE(b);
}

int sendChunkSize(RtmpSession *session, Buffer *buffer, int chunk_size)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    _buildSetChunkSize(b,  chunk_size);

    send(session->conn->fd, buffer->data, bs_pos(b), 0);

    //printfChar(buffer->data, bs_pos(b));

    FREE(b);
}

int sendConnectResult(RtmpSession *session, Buffer *buffer, double transactionId)
{
    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b) 
        return NET_FAIL;

    _buildConnectResult(b, transactionId , RTMP_FMSVER, 
                                    RTMP_CAPABILITIES, "NetConnection.Connect.Success", 
                                    RTMP_LEVEL_STATUS, "Connection succeeded.", 0.0);

    send(session->conn->fd, buffer->data, bs_pos(b), 0);

    //printfChar(buffer->data, bs_pos(b));

    FREE(b);
}