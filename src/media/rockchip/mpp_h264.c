#include "mpp_h264.h"
#include "v4l2_capture.h"
#include "mpp_encode.h"
#include "util.h"
#include "send_chunk.h"

#define FRAME_COUNT 3

VideoMedia *createMppH264Media(RtmpConfig *config)
{
    VideoMedia *media = CALLOC(1, VideoMedia); 
    if (!media)
        return NULL;

    media->display_height = config->display_height;
    media->display_width  = config->display_width;
    media->height         = config->height;
    media->width          = config->width;
    media->fps            = config->fps;

    //MPP_FMT_YUV422_YUYV
    //640, 480, V4L2_PIX_FMT_YUYV, 30
    do {
        media->v4l2 = createV4l2Capture(config->v4l2_device, FRAME_COUNT, media->height, media->width, config->v4l2_format, media->fps);
        if (!media->v4l2)
        {
            ERR("create v4l2 fail");
            break;
        }

        media->ctx = createMppEncode(media->height, media->width, media->fps, config->mpp_format); 
        if (!media->ctx) 
            break;

        media->sps_buffer = getPpsAndSps(media->ctx, NAL_UNIT_TYPE_SPS);
        if (!media->sps_buffer)
        {
            ERR("media->sps_buffer");
            break;
        }

        media->pps_buffer = getPpsAndSps(media->ctx, NAL_UNIT_TYPE_PPS);
        if (!media->pps_buffer )
        {
            ERR("media->pps_buffer");
            break;
        }

        sps_t *sps = read_seq_parameter_set_rbsp(media->sps_buffer);
        if (!sps)
            break;

        media->width         = sps->width;
        media->height        = sps->height;
        media->fps           = sps->fps;
        media->duration      = (int)1000/media->fps; 
        media->level_idc     = sps->level_idc;
        media->profile_idc   = sps->profile_idc;
        media->videodatarate = VIDEODATARATE;
        media->videocodecid  = VIDEOCODECID_H264;

        FREE(sps);
        
        media->avc_sequence = rtmpAvcSequence(media->sps_buffer, media->pps_buffer);
        if (!media->avc_sequence) 
            break;

        LOG("v4l2 stream create success");
        return media;
    } while (0);

    destroyMppH264Media(media);

    return NULL;
}

void destroyMppH264Media(VideoMedia *media)
{
    if (!media)
        return;

    if (media->ctx)
        destroyMppEncode(media->ctx);

    if (media->v4l2)
        destroyV4l2Capture(media->v4l2);

    FREE(media->avc_sequence);
	FREE(media->pps_buffer);
    FREE(media->sps_buffer);
    FREE(media);
}

Buffer *getMppH264MediaFrame(VideoMedia *media)
{
    if (!media)
        return NULL;

    Buffer *buffer = getV4l2Frame(media->v4l2);
    if (!buffer) 
        return NULL;

    Buffer *mpp_buffer = encodeMppFrame(media->ctx, buffer);
    if (!mpp_buffer)
    {
        FREE(buffer);
        return NULL;
    }

    Buffer *rtmp_buffer = rtmpWriteVideoFrame(mpp_buffer->data, 
                                                mpp_buffer->length, 
                                                mpp_buffer->frame_type, 
                                                media->duration);
    FREE(buffer);
    FREE(mpp_buffer);
    return rtmp_buffer;
}