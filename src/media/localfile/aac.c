#include "aac.h"
#include "rtmp_media.h"
#include "send_chunk.h"

static int gSampleRateIndex[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350};
// channelCfg 值	声道数	声道配置描述
// 0	1	单声道（Mono）
// 1	2	立体声（Stereo）
// 2	3	3 声道（如 LCR）
// 3	4	4 声道（如 L R Ls Rs）
// 4	5	5.1 声道
// 5	6	6.1 声道
// 6	7	7.1 声道
// 7	8	8 声道
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
    // LOG("samplingFreqIndex %d, length %d, %d, number %d, channle %d, protectionAbsent %d, profile %d, adtsBufferFullness %d, privateBit %d",
    //     header->samplingFreqIndex, header->aacFrameLength,
    //     bs_pos(b), header->numberOfRawDataBlockInFrame, header->channelCfg, header->protectionAbsent, header->profile, header->adtsBufferFullness, header->privateBit);
    return bs_pos(b);
}

static int _runMediaStream(AudioMedia *media, FifoQueue *frame_queue, Buffer *buffer, AdtsHeader *header)
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

        frame->timestamp = calculateTimeStamp(&media->fractional_part, gSampleRateIndex[header->samplingFreqIndex], 1024);

        memcpy(frame->data, buffer->data + buffer->index + len, frame->length);

        enqueue(frame_queue, rtmpWriteAudioFrame(frame, header->samplingFreqIndex, 1, header->channelCfg));

        buffer->index += header->aacFrameLength;

        return NET_FAIL;
    } 

    return NET_SUCCESS;
}


AudioMedia *createAacMedia(const char *file)
{

    Buffer *buffer = NULL;
    AudioMedia *media  = NULL;

    do {
        buffer = readMediaFile(file);
        if (!buffer)
            break;

        media = CALLOC(1, AudioMedia);
        if (!media)
            break;

        media->queue = createFifiQueue();
        if (!media->queue)
            break;

        AdtsHeader header = {0};

        while (_runMediaStream(media, media->queue, buffer, &header));

        media->adts_sequence = rtmpadtsSequence(header.profile, header.samplingFreqIndex, 1, header.channelCfg);
        if (!media->adts_sequence)
            break;

        media->frame_count = list_count_nodes(&media->queue->list);

        media->stereo = header.channelCfg;
        media->audiocodecid = AUDIOCODECID;
        media->audiodatarate = AUDIODATARATE;
        media->audiosamplerate = gSampleRateIndex[header.samplingFreqIndex];
        media->audiosamplesize = 16;
        media->duration = (int) (1024 * 1000)/media->audiosamplerate;

        FREE(buffer);

        return media;

    } while (0);

   
    destroyAacMedia(media);
    
    return NULL;
}

void destroyAacMedia(AudioMedia *media)
{
    if (!media)
        return;

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
