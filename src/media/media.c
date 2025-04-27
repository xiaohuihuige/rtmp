#include "media.h"
#include "type.h"
#include "h264.h"
#include "send_chunk.h"

static Buffer *_readH264Nalu(Media *media, const char *file_path)
{
    FILE *fp = fopen(file_path, "rb+");
    if (!fp)
        return NULL;

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    Buffer *buffer = createBuffer(fileSize);
    if (!buffer)
        return NULL;

    fread(buffer->data, 1, fileSize, fp);

    fclose(fp);

    return buffer;
}

static void _paserNaluPacket(Media *media, uint8_t *data, int size, int type)
{
    Buffer *frame = createFrameBuffer(data, size, type, 0);
    if (type == NAL_UNIT_TYPE_SPS) {
        if (!media->sps)
            media->sps = frame;
        return;
    }

    if (type == NAL_UNIT_TYPE_PPS) {
        if (!media->pps)
            media->pps = frame;
        return;
    }

    Buffer *frame_buffer = rtmpWriteFrame(frame);
    if (!frame_buffer)
        return;

    FREE(frame);

    enqueue(media->frame_fifo, frame_buffer);

    //LOG("count %d, %d", list_count_nodes(&media->frame_fifo->list), frame_buffer->length);
}

static int _runMediaStream(Media *media, Buffer *buffer)
{
    if (buffer->index >= buffer->length)
        return NET_FAIL;

    int nal_start = 0, nal_end = 0;

    int resp = find_nal_unit(buffer->data + buffer->index, buffer->length - buffer->index, &nal_start, &nal_end);
    if (resp <= 0)
        return NET_FAIL;

    uint8_t *nalu_start = buffer->data + buffer->index + nal_start;

    _paserNaluPacket(media, nalu_start, nal_end - nal_start, (*nalu_start) & 0x1F);

    buffer->index += nal_end - nal_start;

    return NET_SUCCESS;
}

Media *createMediaChannl(const char *app, int stream_type, const char *file_path)
{
    Media *media = CALLOC(1, Media);
    if (!media)
        return NULL;

    Buffer *buffer = _readH264Nalu(media, file_path);
    if (!buffer)
        return NULL;
    
    media->frame_fifo = createFifiQueue();
    if (!media->frame_fifo)
        return NULL;

    while (1) if (_runMediaStream(media, buffer)) break;

    FREE(buffer);

    if (media->pps && media->sps){
        media->avc_buffer = rtmpAvcSequence(media->sps, media->pps);
        if (!media->avc_buffer) 
            return NULL;
    }

    FREE(media->sps);
    FREE(media->pps);

    media->frame_count = list_count_nodes(&media->frame_fifo->list);

    snprintf(media->app, sizeof(media->app), "%s", app);

    LOG("media create success %p, frame count %d, media name %s", media, media->frame_count, media->app);

    return media;
}


Buffer *getMediaStreamFrame(Media *media, int index)
{
    if (media->frame_count <= index)
        return NULL;

    int count = 0;

    FifoQueue *pos = NULL;

    list_for_each_entry(pos, &media->frame_fifo->list, list) 
    {
        if (count == index) {
            return pos->task; // 找到指定索引的数据
        }
        count++;
    }
    
    return NULL; // 如果索引超出范围，返回 NULL
}