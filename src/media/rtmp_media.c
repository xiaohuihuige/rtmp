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

    Buffer *h264_buffer = NULL;
    Buffer *acc_buffer  = NULL;

    if (h264_file) {
        h264_buffer = _readMediaFile(h264_file);
    }

    if (aac_file) {
        acc_buffer = _readMediaFile(aac_file);
    }

    media->video  = createH264Media(h264_buffer);

    media->audio  = createAacMedia(acc_buffer);
    
    return media;
}

void destroyRtmpMedia(RtmpMedia *media)
{
    destroyH264Media(media->video);
    destroyAacMedia(media->audio);
    media->audio = NULL;
    media->video = NULL;
    FREE(media);
}

static int sendVideoFrameToclient(void *args)
{
    RtmpSession *session = (RtmpSession *)args;

    Buffer *frame = getH264MediaFrame(session->media->video, session->channle[VIDEO_CHANNL].index);
    if (!frame) {
        session->channle[VIDEO_CHANNL].index = 0;
        frame = getH264MediaFrame(session->media->video, session->channle[VIDEO_CHANNL].index);
        if (!frame) 
            return NET_SUCCESS;
    }

    if (NET_SUCCESS != sendFrameStream(session, frame, frame->timestamp))
    {
        ERR("send video error");
        return NET_FAIL;
    }

    session->channle[VIDEO_CHANNL].index++;

    return frame->frame_type;
}

static int sendAudioFrameToclient(void *args)
{
    RtmpSession *session = (RtmpSession *)args;

    Buffer *frame = getAacMediaFrame(session->media->audio, session->channle[AUDIO_CHANNL].index);
    if (!frame) {
        session->channle[AUDIO_CHANNL].index = 0;
        return NET_SUCCESS;
    }

    if (NET_SUCCESS != sendAudioStream(session, frame, frame->timestamp))
    {
        ERR("send audio error");
        return NET_FAIL;
    }

    session->channle[AUDIO_CHANNL].index++;

    return frame->frame_type;
}

static void _sendGopVideoFrameToclient(RtmpSession *session, int long_time)
{
    if (!session)
        return ;
    
    int base_time = 0;
    int video_time = 0;
    int audio_time = 0;

    while (base_time <= long_time)
    {
        if (base_time <= video_time && (base_time + 10) > video_time && session->media->video->queue)
        {
            sendVideoFrameToclient(session);
            video_time += session->media->video->duration;
            continue;
        }

        if (base_time <= audio_time && (base_time + 10) > audio_time && session->media->audio->queue)
        {
            sendAudioFrameToclient(session);
            audio_time += session->media->audio->duration;
            continue;
        }
        base_time += 10;
    }
}

int startPushSessionStream(RtmpSession *session)
{
    if (!session->media)
        return NET_FAIL;

    if (session->media->video->queue)
        sendVideoAVCStream(session, session->media->video->avc_sequence, 1000);

    if (session->media->audio->queue)
        sendAudioAdtsStream(session, session->media->audio->adts_sequence, 1000);

    _sendGopVideoFrameToclient(session, 3000);

    if (session->media->video && session->media->video->queue) {
        session->video_task = addTimerTask(session->conn->tcps->scher, 
                                            session->media->video->duration,
                                            session->media->video->duration, 
                                            sendVideoFrameToclient, (void *)session);
        if (!session->video_task)
            return NET_FAIL;
    }

    if (session->media->audio && session->media->audio->queue) {
        session->audio_task = addTimerTask(session->conn->tcps->scher, 
                                            session->media->audio->duration, 
                                            session->media->audio->duration, 
                                            sendAudioFrameToclient, (void *)session);
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