#include "h264.h"
#include "type.h"
#include "send_chunk.h"
#include "h264_sps.h"
#include "h264_nal.h"

typedef struct 
{
    Buffer *pps_buffer;
    Buffer *sps_buffer;
} H264Info;

static void _paserNaluPacket(FifoQueue *frame_queue, uint8_t *data, int size, int type, H264Info *info)
{   
    if (!frame_queue || !data)
        return;

    if (type == NAL_UNIT_TYPE_SPS) {
        FREE(info->sps_buffer);
        info->sps_buffer = createFrameBuffer(data, size, type, 0);
        return;
    } else if (type == NAL_UNIT_TYPE_PPS) {
        FREE(info->pps_buffer);
        info->pps_buffer = createFrameBuffer(data, size, type, 0);
        return;
    } else if (type == NAL_UNIT_TYPE_SEI) {
        return;
    }

    enqueue(frame_queue, rtmpWriteFrame(data, size, type));
}

static int _runMediaStream(FifoQueue *frame_queue, Buffer *buffer, H264Info *info)
{
    if (!buffer || buffer->index >= buffer->length || !frame_queue)
        return NET_FAIL;

    int nal_start = 0, nal_end = 0;

    int resp = find_nal_unit(buffer->data + buffer->index, buffer->length - buffer->index, &nal_start, &nal_end);
    if (resp <= 0)
        return NET_FAIL;

    uint8_t *nalu_start = buffer->data + buffer->index + nal_start;

    int frame_type = (*nalu_start) & 0x1F;

    _paserNaluPacket(frame_queue, nalu_start, nal_end - nal_start, frame_type, info);

    buffer->index += nal_end - nal_start;

    return NET_SUCCESS;
}

Buffer *getH264MediaFrame(VideoMedia *media, int index)
{
    if (media->frame_count  <= index)
        return NULL;

    int count = 0;
    FifoQueue *pos = NULL;

    list_for_each_entry(pos, &media->queue->list, list)  
    {
        if (count == index) {
            return pos->task; // 找到指定索引的数据
        }
        count++;
    }
    
    return NULL; // 如果索引超出范围，返回 NULL
}

int _getGoplength(VideoMedia *media)
{
    int index = 0;
    while (1){
        Buffer *frame = getH264MediaFrame(media, index);
        if (!frame) 
            break;
   
        if (frame->frame_type == NAL_UNIT_TYPE_CODED_SLICE_IDR && index != 0) 
            break;

        index++;
    }

    return index;
}

VideoMedia *createH264Media(Buffer *buffer)
{
    VideoMedia *media = CALLOC(1, VideoMedia); 
    if (!media)
        return NULL;


    H264Info info = {0};

    media->queue = createFifiQueue();
    if (!media->queue)
        return NULL;

    while (1) if (_runMediaStream(media->queue, buffer, &info)) break;

    sps_t *sps = read_seq_parameter_set_rbsp(info.sps_buffer);
    if (!sps)
        return NULL;

    media->width = sps->width;
    media->height = sps->height;
    media->fps = sps->fps;
    media->duration = (int)1000/media->fps; 
    media->level_idc = sps->level_idc;
    media->profile_idc = sps->profile_idc;
    
    media->avc_sequence = rtmpAvcSequence(info.sps_buffer, info.pps_buffer);
    if (!media->avc_sequence) 
        return NULL;

    media->frame_count = list_count_nodes(&media->queue->list);  
    media->gop_size = _getGoplength(media);

    FREE(info.sps_buffer);
    FREE(info.pps_buffer);
    FREE(sps);

    return media;
}

void destroyH264Media(VideoMedia *media)
{
    destroyFifoQueue(media->queue, Buffer);
    FREE(media->avc_sequence);
    FREE(media);
}