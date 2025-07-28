#include "mpp_h264.h"
#include "v4l2_capture.h"
#include "mpp_encode.h"
#include "util.h"
#include "send_chunk.h"

#define FRAME_COUNT 3

typedef struct 
{
    V4l2Capture *v4l2;
    MppContext *ctx;
} MppInfo;

VideoMedia *createMppH264Media(const char *device_name)
{
    VideoMedia *media = CALLOC(1, VideoMedia); 
    if (!media)
        return NULL;

    media->mpp_context = CALLOC(1, MppInfo); 
    if (!media->mpp_context)
        return NULL;

    MppInfo *info = (MppInfo *)media->mpp_context;
    sps_t *sps = NULL;

    do {
        info->v4l2 = createV4l2Capture(device_name, FRAME_COUNT, 640, 480, V4L2_PIX_FMT_YUYV, 30);
        if (!info->v4l2)
        {
            ERR("create v4l2 fail");
            break;
        }

        info->ctx = createMppEncode(640, 480, 30); 
        if (!info->ctx) 
            break;

        media->sps_buffer = getPpsAndSps(info->ctx, NAL_UNIT_TYPE_SPS);
        if (!media->sps_buffer)
        {
            ERR("media->sps_buffer");
            break;
        }

        media->pps_buffer = getPpsAndSps(info->ctx, NAL_UNIT_TYPE_PPS);
        if (!media->pps_buffer )
        {
            ERR("media->pps_buffer");
            break;
        }

        sps = read_seq_parameter_set_rbsp(media->sps_buffer);
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

        return media;

    } while (0);

    destroyMppH264Media(media);

    return NULL;
}

void destroyMppH264Media(VideoMedia *media)
{
    if (!media)
        return;

    MppInfo *mctx = (MppInfo  *)media->mpp_context;
    
    if (mctx && mctx->ctx)
        destroyMppEncode(mctx->ctx);

    destroyV4l2Capture(mctx->v4l2);

    FREE(media->avc_sequence);
	FREE(media->pps_buffer);
    FREE(media->sps_buffer);
    FREE(media->mpp_context);
    FREE(media);
}

Buffer *getMppH264MediaFrame(VideoMedia *media)
{
    MppInfo *mctx = (MppInfo  *)media->mpp_context;
    Buffer *buffer = getV4l2Frame(mctx->v4l2);
    if (!buffer) 
        return NULL;
    
    //static long long start_time = 0;

    Buffer *mpp_buffer = encodeMppFrame(mctx->ctx, buffer);
    // Buffer *rtmp_buffer = rtmpWriteVideoFrame(mpp_buffer->data, 
    //                                         mpp_buffer->length, 
    //                                         mpp_buffer->frame_type, 
    //                                         calculateTimeStamp(&media->fractional_part, media->fps, 1));

    Buffer *rtmp_buffer = rtmpWriteVideoFrame(mpp_buffer->data, 
                                            mpp_buffer->length, 
                                            mpp_buffer->frame_type, 
                                            media->duration);
    //long long end_time = get_time_ms();

    //LOG("%d, %d, %lld", rtmp_buffer->length  , mpp_buffer->frame_type, end_time - start_time);

    //start_time = get_time_ms();

    FREE(buffer);
    FREE(mpp_buffer);

    return rtmp_buffer;
}