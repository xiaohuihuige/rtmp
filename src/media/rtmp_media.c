#include "rtmp_media.h"
#include "send_chunk.h"
#include "rtmp_server.h"

static Buffer *_readMediaFile(const char *file_path)
{
    FILE *fp = fopen(file_path, "rb+");
    if (!fp)
        return NULL;

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    Buffer *buffer = createBuffer(fileSize);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    fread(buffer->data, 1, fileSize, fp);

    fclose(fp);

    return buffer;
}

RtmpMedia *createRtmpMedia(const char *app, const char *h264_file, const char *aac_file)
{
    RtmpMedia *media = CALLOC(1, RtmpMedia);
    if (!media)
        return NULL;

    snprintf(media->app, sizeof(media->app), "%s", app);

    Buffer *h264_buffer = _readMediaFile(h264_file);

    //Buffer *acc_buffer  = _readMediaFile(aac_file);

    media->video = createH264Media(h264_buffer);

    return media;
}

void destroyRtmpMedia(RtmpMedia *media)
{
    destroyH264Media(media->video);
    FREE(media);
}

static int sendFrameToclient(RtmpSession *session)
{
    Buffer *frame = getH264MediaFrame(session->media->video, session->channle[0].index);
    if (!frame) {
        session->channle[0].index = 0;
        return NET_SUCCESS;
    }

    if (frame->frame_type == NAL_UNIT_TYPE_CODED_SLICE_IDR) 
        sendFrameStream(session, session->media->video->avc_sequence, session->channle[0].base_time);

    if (NET_SUCCESS != sendFrameStream(session, frame, session->channle[0].base_time))
        return NET_FAIL;

    session->channle[0].base_time += session->media->video->duration;

    session->channle[0].index++;

    return frame->frame_type;
}

static void _sendStreamGopCache(RtmpSession *session)
{
    while (1) {
        sendFrameToclient(session);
    }
}

static int _pushStreamLoop(void *args)
{
    if (!args)
        return NET_FAIL;

    if (NET_SUCCESS != sendFrameToclient((RtmpSession *)args))
        stopPushSessionstream((RtmpSession *)args);

    return NET_SUCCESS;
}

int startPushSessionStream(RtmpSession *session)
{
    if (!session->media)
        return NET_FAIL;

    _sendStreamGopCache(session);

    session->stream_task = addTimerTask(session->conn->tcps->scher, 0, 
                                        session->media->video->fps, 
                                        _pushStreamLoop, (void *)session);
    if (!session->stream_task)
        return NET_FAIL;

    return NET_SUCCESS;
}

int findRtmpMedia(RtmpSession *session)
{
    if (!session)
        return NET_FAIL;

    session->media = findRtmpServerMedia((RtmpServer *)session->conn->tcps->parent, session->config.app);
    if (!session->media) {
        ERR("find rtmp stream error");
        return NET_FAIL;
    }

    LOG("find stream %p, name %s", session->media, session->media->app);

    return NET_SUCCESS;
}

void stopPushSessionstream(RtmpSession *session)
{
    if (!session->media)
        return;

    if (session->stream_task)
        deleteTimerTask(session->stream_task);

    session->stream_task = NULL;
}