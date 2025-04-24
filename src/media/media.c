#include "media.h"

Media *createMediaChannl(const char *app, int stream_type, const char *file_path)
{
    Media *media = CALLOC(1, Media);
    if (!media)
        return NULL;

    media->fp = fopen(file_path, "rb+");
    if (!media->fp)
        return NULL;

    fseek(media->fp, 0, SEEK_END);
    long fileSize = ftell(media->fp);
    fseek(media->fp, 0, SEEK_SET);

    media->buffer = createBuffer(fileSize);
    if (!media->buffer)
        return NET_FAIL;

    fread(media->buffer->data, 1, fileSize, media->fp);

    fclose(media->fp);

    return media;
}

static void rtmp_paser_packet(uint8_t *data, int size, int type)
{
    LOG("type %d,%d", size, type);
    Buffer *frame = createFrameBuffer(data, size, type, 0);
}

static int _runMediaStream(Media *media)
{
    if (media->buffer->index >= media->buffer->length)
        return NET_FAIL;

    int nal_start = 0, nal_end = 0;

    int resp = find_nal_unit(media->buffer->data + media->buffer->index, media->buffer->length - media->buffer->index, &nal_start, &nal_end);
    if (resp <= 0)
        return NET_FAIL;

    uint8_t *nalu_start = media->buffer->data + media->buffer->index + nal_start;

    rtmp_paser_packet(nalu_start, nal_end - nal_start, (*nalu_start) & 0x1F);

    media->buffer->index += nal_end - nal_start;

    return NET_SUCCESS;
}

int startRunMediaStream(Media *media)
{
  
}


