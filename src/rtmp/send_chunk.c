#include "send_chunk.h"

static int _sendRtmpPacket(RtmpSession *session, HeaderChunk *header, Buffer *frame)
{
    if (!session || !header || !frame || header->length <= 0)
    {
        ERR("args error");
        return NET_FAIL;
    }
 
    int index = frame->index;

    while (frame->length > index) {
        bs_init(session->b, session->buffer->data, session->buffer->length);

        writeChunkHeader(session->b, header);

        int chunk_size = frame->length - index < RTMP_OUTPUT_CHUNK_SIZE ? frame->length - index : RTMP_OUTPUT_CHUNK_SIZE;
        
        bs_write_bytes(session->b, frame->data + index, chunk_size);

        int code = sendToClient(session, session->buffer->data, bs_pos(session->b));
        if (code <= 0)
            return NET_FAIL;

        index += chunk_size;

        header->fmt  = RTMP_CHUNK_TYPE_3;
    }

    return NET_SUCCESS;
}

Buffer *rtmpWriteAudioFrame(Buffer *frame, int sample_rate_index, int sample_size, int channel)
{
    Buffer *buffer = createBuffer(frame->length + 2);
    if (!buffer)
        return NULL;
    
    buffer->timestamp = frame->timestamp;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NULL;

    bs_write_u(b, 4, 0x0A);
    bs_write_u(b, 2, sample_rate_index - 1);
    bs_write_u(b, 1, sample_size);
    bs_write_u(b, 1, channel);

    bs_write_u8(b, 0x01);

    bs_write_bytes(b, frame->data, frame->length);

    FREE(b);

    return buffer;
}

// ① AF 含义 : AAC 格式 , 44100 Hz 采样 , 16 位采样位数 , 立体声 ;
// ② AE 含义 : AAC 格式 , 44100 Hz 采样 , 16 位采样位数 , 单声道
// rtmpPacket->m_body[0] = 0xAF;   //默认立体声
//     if (mChannelConfig == 1) {
//         // 如果是单声道, 将该值修改成 AE
//         rtmpPacket->m_body[0] = 0xAE;
//     }
// https://blog.csdn.net/jctian000/article/details/93205521
// https://www.cnblogs.com/8335IT/p/18208384

// 4bit soundFormat 1010(10)  0xA         AAC
// 2bit(3) soundRate   11        44100Hz
// 1bit(1) soundSize   1         16bit
// 1bit(1) soundType   1         sndstero    0xf

// 8bit AACPaketType     0        AAC sequence header
//5 bits(2):audio oject type      AAC LC
//4 bits(4):sample frequency index  44100Hz
//4 bits(2):channel configuration    stero
//1 bits(0):FrameLengthFlag          每个帧率有1024个采样
//1 bits(0):DependOnCoreCoder
//1 bits(0):extenslonFlag

//11 bits(0x2b7) sync extension type
//5 bits(5) audio oject type
//1 bits(0) sbr present flag

Buffer *rtmpadtsSequence(int profile, int sample_rate_index, int sample_size, int channel)
{

    // sample_rate_index 4, 
    // channel 1, 
    // protectionAbsent 1, 
    // profile 1,

    Buffer *buffer = createBuffer(20);
    if (!buffer)
        return NULL;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NULL;

    bs_write_u(b, 4, 0x0A);
    bs_write_u(b, 2, sample_rate_index - 1);
    bs_write_u(b, 1, sample_size);
    bs_write_u(b, 1, channel);

    bs_write_u8(b, 0x00); //2

    bs_write_u(b, 5, profile + 1); 
    bs_write_u(b, 4, sample_rate_index); 
    bs_write_u(b, 4, channel + 1); 

    bs_write_u(b, 1, 0x00);
    bs_write_u(b, 1, 0x00);
    bs_write_u(b, 1, 0x00); //5 + 5 + 11 + 5

    bs_write_u(b, 11, 0x2b7);
    bs_write_u(b, 5, 0x05);
    bs_write_u8(b, 0x00);

    buffer->length = bs_pos(b);

    FREE(b);

    return buffer;
}

Buffer *rtmpWriteVideoFrame(uint8_t *data, int size, int type, int timestamp)
{
    int length = RTMP_FRAME_HEADER_LENGTH + size;

    Buffer *buffer = createBuffer(length);
    if (!buffer)
        return NULL;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NULL;

    buffer->frame_type = type;
    buffer->timestamp  = timestamp;
    
    if (NAL_UNIT_TYPE_CODED_SLICE_IDR == type)
        bs_write_u8(b, 0x17); //0x27
    else 
        bs_write_u8(b, 0x27); //0x27
    
    bs_write_u8(b, 0x01);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);

    bs_write_u(b, 32, size);

    bs_write_bytes(b, data, size);

    FREE(b);

    return buffer;
}

Buffer *rtmpAvcSequence(Buffer *sps_frame, Buffer *pps_frame)
{
    if (!sps_frame || !pps_frame) {
        ERR("args sps frame, pps frame is null");
        return NULL;
    }

    int length = sps_frame->length + pps_frame->length + RTMP_AVC_HEADER_LENGTH;

    Buffer *buffer = createBuffer(length);
    if (!buffer)
        return NULL;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NULL;

    bs_write_u8(b, 0x17);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);
    bs_write_u8(b, 0x00);

    bs_write_u8(b, 0x01);
    bs_write_u8(b, sps_frame->data[1]);
    bs_write_u8(b, sps_frame->data[2]);
    bs_write_u8(b, sps_frame->data[3]);
    bs_write_u8(b, 0xff);

    bs_write_u8(b, 0xe1);
    bs_write_u(b, 16, sps_frame->length);
    bs_write_bytes(b, sps_frame->data, sps_frame->length);
    
    bs_write_u8(b, 0x01);
    bs_write_u(b, 16, pps_frame->length);
    bs_write_bytes(b, pps_frame->data, pps_frame->length);

    FREE(b);
    
    return buffer;
}


int sendFrameStream(RtmpSession *session, Buffer *frame, uint32_t delta)
{
    // HeaderChunk header = {
    //     .fmt = RTMP_CHUNK_TYPE_1,
    //     .csid = RTMP_CHANNEL_DATA1,
    //     .timestamp = delta,
    //     .length = frame->length,
    //     .type_id = RTMP_TYPE_VIDEO,
    // };

    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_DATA,
        .timestamp =  delta,
        .length = frame->length,
        .type_id = RTMP_TYPE_VIDEO,
        .stream_id = 0,
    };

    return _sendRtmpPacket(session, &header, frame);
}

int sendVideoAVCStream(RtmpSession *session, Buffer *frame, uint32_t timestamp)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_DATA,
        .timestamp = timestamp,
        .length = frame->length,
        .type_id = RTMP_TYPE_VIDEO,
        .stream_id = 0,
    };

    return _sendRtmpPacket(session, &header, frame);
}

int sendAudioStream(RtmpSession *session, Buffer *frame, uint32_t delta)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_1,
        .csid = RTMP_CHANNEL_DATA,
        .timestamp = delta,
        .length = frame->length,
        .type_id = RTMP_TYPE_AUDIO,
    };

    return _sendRtmpPacket(session, &header, frame);
}

int sendAudioAdtsStream(RtmpSession *session, Buffer *frame, uint32_t base_time)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_DATA,
        .timestamp = base_time,
        .length = frame->length,
        .type_id = RTMP_TYPE_AUDIO,
        .stream_id = 0,
    };

    return _sendRtmpPacket(session, &header, frame);
}


