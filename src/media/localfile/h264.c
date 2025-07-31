#include "h264.h"
#include "type.h"
#include "send_chunk.h"
#include "h264_sps.h"
#include "h264_nal.h"
#include "util.h"

Buffer *getH264MediaFrame(VideoMedia *media)
{
    if (!media)
        return NULL;

    Buffer *nalu_buffer = NULL;
    while (1)
    {
        nalu_buffer = find_file_nal_unit(media->file_fp);
        if (!nalu_buffer) {
            fseek(media->file_fp, 0, SEEK_SET);
            continue;
        }

        if (nalu_buffer->frame_type == NAL_UNIT_TYPE_SPS 
            || nalu_buffer->frame_type == NAL_UNIT_TYPE_PPS 
            || nalu_buffer->frame_type == NAL_UNIT_TYPE_SEI) {
            FREE(nalu_buffer);
            continue;
        } 
        break;
    }

    //LOG("nalu_buffer->length %d", nalu_buffer->length);
    Buffer *buffer = rtmpWriteVideoFrame(nalu_buffer->data, 
                                        nalu_buffer->length, 
                                        nalu_buffer->frame_type, 
                                         media->duration);
    
    FREE(nalu_buffer);
    if (!buffer)
        return NULL;
    return buffer;
}

VideoMedia *createH264Media(RtmpConfig *config)
{
    if (!config->h264_file  || access(config->h264_file,  R_OK | F_OK))
        return NULL;

    VideoMedia *media = CALLOC(1, VideoMedia); 
    if (!media) 
        return NULL;

    media->file_fp = fopen(config->h264_file, "rb+");
    if (!media->file_fp)
       return NULL;

    while (1)
    {
        Buffer * buffer = find_file_nal_unit(media->file_fp);
        if (!buffer)
            break;

        if (buffer->frame_type == NAL_UNIT_TYPE_SPS) {
            media->sps_buffer = buffer;
        } else if (buffer->frame_type == NAL_UNIT_TYPE_PPS) {
            media->pps_buffer = buffer;
        } else {
            FREE(buffer);
        }

        if (media->pps_buffer && media->sps_buffer)
            break;
    }

    //LOG("media->pps_buffer");

    if (!media->avc_sequence && media->sps_buffer && media->pps_buffer)
    {
        sps_t *sps = read_seq_parameter_set_rbsp(media->sps_buffer);
        if (!sps)
            return NULL;

        media->width         = sps->width;
        media->height        = sps->height;
        media->fps           = sps->fps;
        media->duration      = (int)1000/media->fps; 
        media->level_idc     = sps->level_idc;
        media->profile_idc   = sps->profile_idc;
        media->videodatarate = VIDEODATARATE;
        media->videocodecid  = VIDEOCODECID_H264;
        media->display_height = sps->height;
        media->display_width  = sps->width;

        LOG("media->width %d, media->height %d, media->fps %d", media->width, media->height, media->fps);

        media->avc_sequence = rtmpAvcSequence(media->sps_buffer, media->pps_buffer);
        if (!media->avc_sequence) 
            return NULL;

        LOG("h264 init success");

        FREE(sps);

        return media;
    }
    LOG("media->pps_buffer");
    destroyH264Media(media);
    return NULL;
}

void destroyH264Media(VideoMedia *media)
{
    if (!media)
        return;
    fclose(media->file_fp);
    FREE(media->avc_sequence);
    FREE(media);
}