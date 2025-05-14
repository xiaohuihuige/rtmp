#include "aac.h"
#include "rtmp_media.h"
#include "send_chunk.h"

static int paresADTSHeader(AdtsHeader *header, uint8_t *data, int size)
{
    bs_t *b = bs_new(data, size);

    header->syncword = bs_read_u(b, 12);

    header->id = bs_read_u(b, 1);
    header->layer = bs_read_u(b, 2);
    header->protectionAbsent = bs_read_u(b, 1);
    header->profile = bs_read_u(b, 2);
    header->samplingFreqIndex = bs_read_u(b, 4);
    header->privateBit = bs_read_u(b, 1);
    header->channelCfg = bs_read_u(b, 3);
    header->originalCopy = bs_read_u(b, 1);
    header->home = bs_read_u(b, 1);

    header->copyrightIdentificationBit = bs_read_u(b, 1);
    header->copyrightIdentificationStart = bs_read_u(b, 1);

    header->aacFrameLength = bs_read_u(b, 13);
    header->adtsBufferFullness = bs_read_u(b, 11);

    header->numberOfRawDataBlockInFrame = bs_read_u(b, 2);
    header->channelCfg = bs_read_u(b, 3);
    LOG("samplingFreqIndex %d, length %d, %d, number %d, channle %d, protectionAbsent %d, profile %d, adtsBufferFullness %d",
        header->samplingFreqIndex, header->aacFrameLength,
        bs_pos(b), header->numberOfRawDataBlockInFrame, header->channelCfg, header->protectionAbsent, header->profile, header->adtsBufferFullness);
    return bs_pos(b);
}

static int _runMediaStream(FifoQueue *frame_queue, Buffer *buffer, AdtsHeader *header)
{
    if (buffer->index >= buffer->length)
        return NET_SUCCESS;

    if (!buffer || !frame_queue)
        return NET_FAIL;

    if ((buffer->data + buffer->index)[0] == 0xFF && ((buffer->data + buffer->index)[1] & 0xF0) == 0xF0)
    {
        int len = paresADTSHeader(header, buffer->data + buffer->index, buffer->length - buffer->index);
        if (len <= 0)
            return NET_FAIL;

        Buffer *frame = createBuffer(header->aacFrameLength - len);
        if (!frame)
            return NET_FAIL;

        memcpy(frame->data, buffer->data + buffer->index + len, frame->length);

        enqueue(frame_queue, rtmpWriteAudioFrame(frame, header->samplingFreqIndex));

        buffer->index += header->aacFrameLength;

        return NET_FAIL;
    } 

    return NET_SUCCESS;
}

AudioMedia *createAacMedia(Buffer *buffer)
{
    AudioMedia *media = CALLOC(1, AudioMedia);
    if (!media)
        return NULL;

    media->queue = createFifiQueue();
    if (!media->queue)
        return NULL;

    AdtsHeader header = {0};

    while (_runMediaStream(media->queue, buffer, &header));

    media->adts_sequence = rtmpadtsSequence(header.profile, header.samplingFreqIndex, header.channelCfg);
    if (!media->adts_sequence)
        return NULL;

    media->frame_count = list_count_nodes(&media->queue->list);

    //media->gop_size = _getGoplength(media);

    media->duration = 20;

    media->stereo = header.channelCfg;
    media->audiocodecid = AUDIOCODECID;
    media->audiodatarate = AUDIODATARATE;
    media->audiosamplerate = 44100;
    media->audiosamplesize = 16;

    return media;
}

void destroyAacMedia(AudioMedia *media)
{
    destroyFifoQueue(media->queue, Buffer);
    FREE(media->adts_sequence);
    FREE(media);
}

Buffer *getAacMediaFrame(AudioMedia *media, int index)
{
    if (media->frame_count <= index)
        return NULL;

    int count = 0;
    FifoQueue *pos = NULL;

    list_for_each_entry(pos, &media->queue->list, list)
    {
        if (count == index)
        {
            return pos->task; // 找到指定索引的数据
        }
        count++;
    }

    return NULL; // 如果索引超出范围，返回 NULL
}
