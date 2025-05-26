#include "h264.h"
#include "type.h"
#include "send_chunk.h"
#include "h264_sps.h"
#include "h264_nal.h"
#include "util.h"

static void _paserNaluPacket(VideoMedia *media, uint8_t *data, int size, int type)
{   
    if (!media->queue || !data)
        return;

    if (type == NAL_UNIT_TYPE_SPS && !media->sps_buffer) {
        media->sps_buffer = createFrameBuffer(data, size, type, 0);
        return;
    } else if (type == NAL_UNIT_TYPE_PPS && !media->pps_buffer) {
        media->pps_buffer = createFrameBuffer(data, size, type, 0);
        return;
    } else if (type == NAL_UNIT_TYPE_SEI) {
        return;
    }

    if (!media->avc_sequence && media->sps_buffer && media->pps_buffer)
    {
        sps_t *sps = read_seq_parameter_set_rbsp(media->sps_buffer);
        if (!sps)
            return;

        media->width         = sps->width;
        media->height        = sps->height;
        media->fps           = sps->fps;
        media->duration      = (int)1000/media->fps; 
        media->level_idc     = sps->level_idc;
        media->profile_idc   = sps->profile_idc;
        media->videodatarate = VIDEODATARATE;
        media->videocodecid  = VIDEOCODECID_H264;

        media->avc_sequence = rtmpAvcSequence(media->sps_buffer, media->pps_buffer);
        if (!media->avc_sequence) 
            return;

        FREE(sps);
    }

    enqueue(media->queue, rtmpWriteVideoFrame(data, size, type, calculateTimeStamp(&media->fractional_part, media->fps, 1)));
}

static int _runMediaStream(VideoMedia *media, Buffer *buffer)
{
    if (!buffer || buffer->index >= buffer->length || !media->queue)
        return NET_FAIL;

    int nal_start = 0, nal_end = 0;

    int resp = find_nal_unit(buffer->data + buffer->index, buffer->length - buffer->index, &nal_start, &nal_end);
    if (resp <= 0)
        return NET_FAIL;

    uint8_t *nalu_start = buffer->data + buffer->index + nal_start;

    int frame_type = (*nalu_start) & 0x1F;

    _paserNaluPacket(media, nalu_start, nal_end - nal_start, frame_type);

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

VideoMedia *createH264Media(const char *file)
{
    Buffer *buffer    = NULL;
    VideoMedia *media = NULL;

    do {
        buffer = readMediaFile(file);
        if (!buffer)
            break;

        media = CALLOC(1, VideoMedia); 
        if (!media) 
            break;

        media->queue = createFifiQueue();
        if (!media->queue)
            break;

        while (1) if (_runMediaStream(media, buffer)) break;

        media->frame_count = list_count_nodes(&media->queue->list);  

        FREE(buffer);
        
        return media;
    } while (0);
    
    FREE(buffer);
    destroyH264Media(media);

    return NULL;

}

void destroyH264Media(VideoMedia *media)
{
    if (!media)
        return;

    if (media->queue)
        destroyFifoQueue(media->queue, Buffer);

    FREE(media->sps_buffer);
    FREE(media->pps_buffer); 
    FREE(media->avc_sequence);
    FREE(media);
}