#include "rtmp_media.h"
#include "send_chunk.h"
#include "rtmp_server.h"
#include "aac.h"

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

    Buffer *acc_buffer  = _readMediaFile(aac_file);

    media->video = createH264Media(h264_buffer);

    media->audio = createAacMedia(acc_buffer);

    return media;
}

void destroyRtmpMedia(RtmpMedia *media)
{
    destroyH264Media(media->video);
    destroyAacMedia(media->audio);
    FREE(media);
}

static int sendVideoFrameToclient(RtmpSession *session)
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

static int sendAudioFrameToclient(RtmpSession *session)
{
    Buffer *frame = getAacMediaFrame(session->media->audio, session->channle[1].index);
    if (!frame)
    {
        session->channle[1].index = 0;
        return NET_SUCCESS;
    }

    if (NET_SUCCESS != sendAudioStream(session, frame, session->channle[1].base_time))
        return NET_FAIL;

    session->channle[1].base_time += session->media->audio->duration;

    session->channle[1].index++;

    return NET_SUCCESS;
}

static void _sendStreamGopCache(RtmpSession *session)
{
    //sendAudioStream(session, session->media->audio->adts_sequence, session->channle[1].base_time);

    int length = session->media->video->gop_size;
    if (length <= 0)
        return;

    while (length--) sendVideoFrameToclient(session);
}

static int _pushVideoStreamLoop(void *args)
{
    if (!args)
        return NET_FAIL;

    sendVideoFrameToclient((RtmpSession *)args);

    return NET_SUCCESS;
}

static int _pushAudioStreamLoop(void *args)
{
    if (!args)
        return NET_FAIL;

    sendAudioFrameToclient((RtmpSession *)args);

    return NET_SUCCESS;
}

int startPushSessionStream(RtmpSession *session)
{
    if (!session->media)
        return NET_FAIL;

    _sendStreamGopCache(session);

    if (session->media->video) {
        session->video_task = addTimerTask(session->conn->tcps->scher, 
                                            session->media->video->duration, 
                                            session->media->video->duration, 
                                            _pushVideoStreamLoop, (void *)session);
        if (!session->video_task)
            return NET_FAIL;
    }

    if (session->media->audio) {
        session->audio_task = addTimerTask(session->conn->tcps->scher, 
                                            session->media->audio->duration, 
                                            session->media->audio->duration, 
                                            _pushAudioStreamLoop, (void *)session);
        if (!session->audio_task)
            return NET_FAIL;
    }
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

    if (session->video_task)
        deleteTimerTask(session->video_task);

    if (session->audio_task)
        deleteTimerTask(session->audio_task);

    session->video_task = NULL;
    session->audio_task = NULL;
}