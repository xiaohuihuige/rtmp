#include "h264.h"
#include "type.h"
#include "send_chunk.h"
#include "h264_sps.h"
#include "h264_nal.h"

static void _paserNaluPacket(FifoQueue *frame_queue, uint8_t *data, int size, int type, H264Info *h264_info)
{   
    if (!frame_queue || !data)
        return;

    Buffer *frame = createFrameBuffer(data, size, type, 0);
    if (!frame)
        return;

    if (type == NAL_UNIT_TYPE_SPS) {
        FREE(h264_info->sps);
        h264_info->sps = frame;
        return;
    }

    if (type == NAL_UNIT_TYPE_PPS) {
        FREE(h264_info->pps);
        h264_info->pps = frame;
        return;
    }

    Buffer *frame_buffer = rtmpWriteFrame(frame);
    if (!frame_buffer) {
        FREE(frame);
        return;
    }

    FREE(frame);

    enqueue(frame_queue, frame_buffer);
}

static int _runMediaStream(FifoQueue *frame_queue, Buffer *buffer, H264Info *h264_info)
{
    if (!buffer || buffer->index >= buffer->length || !frame_queue || !h264_info)
        return NET_FAIL;

    int nal_start = 0, nal_end = 0;

    int resp = find_nal_unit(buffer->data + buffer->index, buffer->length - buffer->index, &nal_start, &nal_end);
    if (resp <= 0)
        return NET_FAIL;

    uint8_t *nalu_start = buffer->data + buffer->index + nal_start;

    _paserNaluPacket(frame_queue, nalu_start, nal_end - nal_start, (*nalu_start) & 0x1F, h264_info);

    buffer->index += nal_end - nal_start;

    return NET_SUCCESS;
}

void *createH264Media(Buffer *buffer)
{
    H264Media *media = CALLOC(1, H264Media); 
    if (!media)
        return NULL;

    H264Info h264_info = {0};

    media->frame_fifo = createFifiQueue();
    if (!media->frame_fifo)
        goto error;

    while (1) if (_runMediaStream(media->frame_fifo, buffer, &h264_info)) break;

    media->sps = read_seq_parameter_set_rbsp(h264_info.sps);

    media->avc_buffer = rtmpAvcSequence(h264_info.sps, h264_info.pps);
    if (!media->avc_buffer) 
        goto error;

    media->frame_count = list_count_nodes(&media->frame_fifo->list);  

    LOG("media create success %p, frame count %d", media, media->frame_count);
    
    FREE(h264_info.sps);
    FREE(h264_info.pps);
    return media;

error:
    FREE(h264_info.sps);
    FREE(h264_info.pps);
    destroyFifoQueue(media->frame_fifo, Buffer);
    FREE(media->sps);
    FREE(media->avc_buffer);
    FREE(media);
    return NULL;
}

void destroyH264Media(void *media)
{
    if (!media)
        return;

    destroyFifoQueue(((H264Media *)media)->frame_fifo, Buffer);
    FREE(((H264Media *)media)->avc_buffer);
    FREE(((H264Media *)media)->sps);
    FREE(media);
}

Buffer *getH264MediaFrame(void *media, int index)
{
    
    if (((H264Media *)media)->frame_count <= index)
        return NULL;

    int count = 0;
    FifoQueue *pos = NULL;

    list_for_each_entry(pos, &((H264Media *)media)->frame_fifo->list, list)  
    {
        if (count == index) {
            return pos->task; // 找到指定索引的数据
        }
        count++;
    }
    
    return NULL; // 如果索引超出范围，返回 NULL
}

Buffer *getH264MediaAVC(void *media)
{
    if (!media)
        return NULL;

    return ((H264Media *)media)->avc_buffer;
}

void *getH264MediaConfig(void *media)
{
    if (!media)
        return NULL;

    return ((H264Media *)media)->sps;
}