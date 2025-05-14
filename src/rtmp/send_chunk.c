#include "send_chunk.h"

Buffer *rtmpWriteAudioFrame(Buffer *frame, int samplingFreqIndex)
{
    Buffer *buffer = createBuffer(frame->length + 2);
    if (!buffer)
        return NULL;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NULL;

    bs_write_u(b, 8, 0xAF);

    // bs_write_u(b, 4, 0x0A);
    // bs_write_u(b, 2, samplingFreqIndex - 1);
    // bs_write_u(b, 1, 0x01);
    // bs_write_u(b, 1, 0x01);

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
// 2bit soundRate   11        44100Hz
// 1bit soundSize   1         16bit
// 1bit soundType   1         sndstero    0xf

// 8bit AACPaketType     0        AAC sequence header
// 5bit audioOjbectType  00010    AAC LC
// 4bit SampleRateIndex  0100     44100Hz
// 4bit ChannelConfig    0010     stero
// 1bit FrameLengthFlag   0       每个帧率有1024个采样
// 1bit DependOnCoreCoder 0
// 1bit extenslonFlag     0

// 0x210
// packet.body[0] = 0xAF; // 固定头
// packet.body[1] = 0x01; // 数据类型
// memcpy(packet.body + 2, aac_data, data_len);
// https://zhuanlan.zhihu.com/p/666954710
Buffer *rtmpadtsSequence(int profile, int samplingFreqIndex, int channelCfg)
{
    Buffer *buffer = createBuffer(4);
    if (!buffer)
        return NULL;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NULL;

    bs_write_u(b, 8, 0xAF);
    // bs_write_u(b, 4, 0x0A);
    // bs_write_u(b, 2, samplingFreqIndex - 1);
    // bs_write_u(b, 1, 0x01);
    // bs_write_u(b, 1, 0x01);

    bs_write_u8(b, 0x00);

    bs_write_u(b, 5, profile + 1);
    bs_write_u(b, 4, samplingFreqIndex);
    bs_write_u(b, 4, channelCfg);

    bs_write_u(b, 1, 0x00);
    bs_write_u(b, 1, 0x00);
    bs_write_u(b, 1, 0x00);

    FREE(b);

    return buffer;
}

Buffer *rtmpWriteVideoFrame(uint8_t *data, int size, int type)
{
    int length = RTMP_FRAME_HEADER_LENGTH + size;

    Buffer *buffer = createBuffer(length);
    if (!buffer)
        return NULL;

    bs_t *b = bs_new(buffer->data, buffer->length);
    if (!b)
        return NULL;

    buffer->frame_type = type;

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


int sendFrameStream(RtmpSession *session, Buffer *frame, uint32_t timestamp)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_0,
        .csid = RTMP_CHANNEL_DATA,
        .timestamp = timestamp,
        .length = frame->length,
        .type_id = RTMP_TYPE_VIDEO,
        .stream_id = 0,
    };

    return sendRtmpPacket(session, &header, frame);
}

int sendAudioStream(RtmpSession *session, Buffer *frame, uint32_t timestamp)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_1,
        .csid = RTMP_CHANNEL_AUDIO,
        .timestamp = timestamp,
        .length = frame->length,
        .type_id = RTMP_TYPE_AUDIO,
        .stream_id = 0,
    };

    return sendRtmpPacket(session, &header, frame);
}

int sendScriptStream(RtmpSession *session, Buffer *frame, uint32_t timestamp)
{
    HeaderChunk header = {
        .fmt = RTMP_CHUNK_TYPE_1,
        .csid = RTMP_CHANNEL_INVOKE,
        .timestamp = timestamp,
        .length = frame->length,
        .type_id = RTMP_TYPE_DATA,
        .stream_id = 0,
    };

    return sendRtmpPacket(session, &header, frame);
}

int sendRtmpPacket(RtmpSession *session, HeaderChunk *header, Buffer *frame)
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


