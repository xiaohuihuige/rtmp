#include "mpp_encode.h"
#include "util.h"

static void _initMppEncodeConfig(MppContext * ctx, int width, int height, int fps, int format)
{
    ctx->width  = width;
    ctx->height = height;
    ctx->fps    = fps;

    //mpp编码图像的行和列都是按16位对齐的，如果输出的行列不是16的整数，则需要在编码时将数据按照16位对齐。
    //此函数就是为了得到行列补齐16整除的数据，比如行是30，通过MPP_ALIGN（30，16）；的输出就是32；
    ctx->hor_stride = 2 * MPP_ALIGN(ctx->width, 16);
    ctx->ver_stride = 2 * MPP_ALIGN(ctx->height, 16);//MPP_ALIGN(ctx->height, 16);
    //实测高度是360的时候，也可以正常运行，高度不用是16的倍数
    //经测试，只有MPP_FMT_YUV420SP(Y+UV交替)和MPP_FMT_YUV420P(Y+U+V)才可以
    ctx->fmt  = format; 
    ctx->type = MPP_VIDEO_CodingAVC;//MPP_VIDEO_CodingAVC;

    switch (ctx->fmt & MPP_FRAME_FMT_MASK)
    {
        case MPP_FMT_YUV420SP:
        case MPP_FMT_YUV420P: {
            ctx->frame_size = ctx->hor_stride *ctx->ver_stride * 3 / 2;
        } break;

        case MPP_FMT_YUV422_YUYV :
        case MPP_FMT_YUV422_YVYU :
        case MPP_FMT_YUV422_UYVY :
        case MPP_FMT_YUV422_VYUY :
        case MPP_FMT_YUV422P :
        case MPP_FMT_YUV422SP : {
            ctx->frame_size = ctx->hor_stride * ctx->ver_stride * 2;
            //ctx->frame_size = MPP_ALIGN(ctx->hor_stride, 64) * MPP_ALIGN(ctx->ver_stride, 64) * 2;
        } break;

        case MPP_FMT_RGB444 :
        case MPP_FMT_BGR444 :
        case MPP_FMT_RGB555 :
        case MPP_FMT_BGR555 :
        case MPP_FMT_RGB565 :
        case MPP_FMT_BGR565 :
        case MPP_FMT_RGB888 :
        case MPP_FMT_BGR888 :
        case MPP_FMT_RGB101010 :
        case MPP_FMT_BGR101010 :
        case MPP_FMT_ARGB8888 :
        case MPP_FMT_ABGR8888 :
        case MPP_FMT_BGRA8888 :
        case MPP_FMT_RGBA8888 : {
            ctx->frame_size = MPP_ALIGN(ctx->hor_stride, 64) * MPP_ALIGN(ctx->ver_stride, 64);
        } break;

        default: {
            ctx->frame_size = MPP_ALIGN(ctx->hor_stride, 64) * MPP_ALIGN(ctx->ver_stride, 64) * 4;
        } break;
    }
}


static int _initMppEncodeQuality(MppContext * ctx, EncodeConfig *config)
{
    MPP_RET ret;
    //RK_U32 rotation  = 0;
    //RK_U32 mirroring = 0;
    //RK_U32 flip      = 0;

    MppEncRefCfg ref = NULL;
    //RK_U32 constraint_set = 0;

    config->gop_mode    = 3;
    config->gop_len     = ctx->fps * 3;
    config->qp_max_i    = 51;
    config->qp_min_i    = 10;

    config->qp_delta_ip = 2;

    config->qp_init     = 30;
    config->qp_max      = 40;
    config->qp_min      = 20;

    config->fps_in_flex = 1;
    config->fps_in_den  = 1;
    config->fps_in_num  = 30;

    config->fps_out_flex = 1;
    config->fps_out_den = 1;
    config->fps_out_num = 30;

    config->bps         = ctx->width * ctx->height / 8 * ctx->fps;//压缩后每秒视频的bit位大小
    config->bps_max     = 5000000;
    config->bps_min     = 3000000;
    config->rc_mode     = MPP_ENC_RC_MODE_VBR;

    /* setup preprocess parameters */
    mpp_enc_cfg_set_s32(ctx->cfg, "prep:width", ctx->width);
    mpp_enc_cfg_set_s32(ctx->cfg, "prep:height", ctx->height);
    mpp_enc_cfg_set_s32(ctx->cfg, "prep:hor_stride", ctx->hor_stride);
    mpp_enc_cfg_set_s32(ctx->cfg, "prep:ver_stride", ctx->ver_stride);
    mpp_enc_cfg_set_s32(ctx->cfg, "prep:format", ctx->fmt);
    mpp_enc_cfg_set_s32(ctx->cfg, "prep:range", MPP_FRAME_RANGE_JPEG);

    // mpp_enc_cfg_set_s32(ctx->cfg, "prep:mirroring", mirroring);
    // mpp_enc_cfg_set_s32(ctx->cfg, "prep:rotation", rotation);
    // mpp_enc_cfg_set_s32(ctx->cfg, "prep:flip", flip);

    /* setup rate control parameters */
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:mode", config->rc_mode);
    mpp_enc_cfg_set_u32(ctx->cfg, "rc:max_reenc_times", 0);
    mpp_enc_cfg_set_u32(ctx->cfg, "rc:super_mode", 0);

    /* fix input / output frame rate */
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:fps_in_flex", config->fps_in_flex);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:fps_in_num", config->fps_in_num);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:fps_in_denom", config->fps_in_den);

    mpp_enc_cfg_set_s32(ctx->cfg, "rc:fps_out_flex", config->fps_out_flex);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:fps_out_num", config->fps_out_num);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:fps_out_denom", config->fps_out_den);

    /* drop frame or not when bitrate overflow */
    mpp_enc_cfg_set_u32(ctx->cfg, "rc:drop_mode", MPP_ENC_RC_DROP_FRM_DISABLED);
    mpp_enc_cfg_set_u32(ctx->cfg, "rc:drop_thd", 20);        /* 20% of max bps */
    mpp_enc_cfg_set_u32(ctx->cfg, "rc:drop_gap", 1);         /* Do not continuous drop frame */

    /* setup bitrate for different rc_mode */
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:bps_target", config->bps);

    mpp_enc_cfg_set_s32(ctx->cfg, "rc:bps_max", config->bps_max ? config->bps_max : config->bps * 17 / 16);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:bps_min", config->bps_min ? config->bps_min : config->bps * 1 / 16);

    mpp_enc_cfg_set_s32(ctx->cfg, "rc:qp_init", config->qp_init ? config->qp_init : -1);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:qp_max", config->qp_max ? config->qp_max : 51);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:qp_min", config->qp_min ? config->qp_min : 10);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:qp_max_i", config->qp_max_i ? config->qp_max_i : 51);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:qp_min_i", config->qp_min_i ? config->qp_min_i : 10);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:qp_ip", 2);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:fqp_min_i", 10);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:fqp_max_i", 45);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:fqp_min_p", 10);
    mpp_enc_cfg_set_s32(ctx->cfg, "rc:fqp_max_p", 45);

    mpp_enc_cfg_set_s32(ctx->cfg, "codec:type", ctx->type);
    mpp_enc_cfg_set_s32(ctx->cfg, "h264:profile", 66);
    mpp_enc_cfg_set_s32(ctx->cfg, "h264:level", 40);
    mpp_enc_cfg_set_s32(ctx->cfg, "h264:cabac_en", 1);
    mpp_enc_cfg_set_s32(ctx->cfg, "h264:cabac_idc", 0);
    mpp_enc_cfg_set_s32(ctx->cfg, "h264:trans8x8", 1);
    // mpp_enc_cfg_set_s32(ctx->cfg, "split:mode", MPP_ENC_SPLIT_NONE);
    // mpp_enc_cfg_set_s32(ctx->cfg, "split:arg", 0);

    mpp_enc_cfg_set_s32(ctx->cfg, "rc:gop", config->gop_len);

    ret = ctx->mpi->control(ctx->ctx, MPP_ENC_SET_CFG, ctx->cfg);
    if (ret) {
        ERR("mpi control enc set cfg failed ret %d", ret);
        return -1;
    }

    RcApiBrief rc_api_brief;
    rc_api_brief.type = ctx->type;
    rc_api_brief.name = (config->rc_mode == MPP_ENC_RC_MODE_SMTRC) ?
                            "smart" : "default";

    ret = ctx->mpi->control(ctx->ctx, MPP_ENC_SET_RC_API_CURRENT, &rc_api_brief);
    if (ret) {
        ERR("mpi control enc set rc api failed ret %d", ret);
        return -1;
    }

    if (ref)
        mpp_enc_ref_cfg_deinit(&ref);

    config->sei_mode =  MPP_ENC_SEI_MODE_DISABLE;
    ret = ctx->mpi->control(ctx->ctx, MPP_ENC_SET_SEI_CFG, &config->sei_mode);
    if (ret) {
        ERR("mpi control enc set sei cfg failed ret %d", ret);
        return -1;
    }

    config->header_mode = MPP_ENC_HEADER_MODE_EACH_IDR;
    ret = ctx->mpi->control(ctx->ctx, MPP_ENC_SET_HEADER_MODE, &config->header_mode);
    if (ret) {
        ERR("mpi control enc set header mode failed ret %d", ret);
        return -1;
    }

    return 0;
}

static void _initEncodeBuffer(MppContext *ctx)
{
    ctx->buf_ptr = mpp_buffer_get_ptr(ctx->frm_buf);
    if (mpp_frame_init(&ctx->frame))
    {
        ERR("mpp_frame_init failed");
        return;
    }

    mpp_frame_set_width(ctx->frame, ctx->width);
    mpp_frame_set_height(ctx->frame, ctx->height);
    mpp_frame_set_hor_stride(ctx->frame, ctx->hor_stride);
    mpp_frame_set_ver_stride(ctx->frame, ctx->ver_stride);
    mpp_frame_set_fmt(ctx->frame, ctx->fmt);
    mpp_frame_set_buffer(ctx->frame, ctx->frm_buf);
    mpp_frame_set_eos(ctx->frame, ctx->frm_eos);
}

MppContext *createMppEncode(int width, int height, int fps, int format)
{
    MPP_RET ret = MPP_OK;
    MppContext * ctx = CALLOC(1, MppContext);
    if (!ctx)
        return NULL;

    do {
        
        _initMppEncodeConfig(ctx, width, height, fps, format);

        ret = mpp_buffer_get(NULL, &ctx->frm_buf, ctx->frame_size);
        if (ret)
        {
            ERR("failed to get buffer for input frame ret");
            break;
        }

        ret = mpp_create(&ctx->ctx, &ctx->mpi);
        if (ret)
        {
            ERR("mpp_create failed");
            break;
        }

        DBG("%p mpp encoder config width: %d, height: %d, type: %d",
             ctx->ctx, ctx->width, ctx->height, ctx->type);

        MppPollType timeout = MPP_POLL_BLOCK;
        ret = ctx->mpi->control(ctx->ctx, MPP_SET_OUTPUT_TIMEOUT, &timeout);
        if (MPP_OK != ret) {
            ERR("mpi control set output timeout %d ret %d", timeout, ret);
            break;
        }

        ret = mpp_init(ctx->ctx, MPP_CTX_ENC, ctx->type);
        if (ret)
        {
            ERR("mpp_init failed ret");
            break;
        }

        ret = mpp_enc_cfg_init(&ctx->cfg);
        if (ret) {
            ERR("mpp_enc_cfg_init failed");
            break;
        }

        ret = ctx->mpi->control(ctx->ctx, MPP_ENC_GET_CFG, ctx->cfg);
        if (ret) {
            ERR("get enc cfg failed");
            break;
        }

        if (_initMppEncodeQuality(ctx, &ctx->config))
            break;

        _initEncodeBuffer(ctx);

        DBG("mpp encode success type :%d", ctx->type);
        return ctx;
    } while (0);

    destroyMppEncode(ctx);

    return NULL;
}

void destroyMppEncode(MppContext * ctx)
{
    if (ctx->mpi->reset(ctx->ctx))
        ERR("mpi->reset failed");

    if (ctx->ctx)
        mpp_destroy(ctx->ctx);
 
    if (ctx->frm_buf)
        mpp_buffer_put(ctx->frm_buf);

    ctx->frm_buf = NULL;
    ctx->ctx = NULL;

    FREE(ctx);
}

Buffer *encodeMppFrame(MppContext * ctx, Buffer *in_buffer)
{
    MPP_RET ret = MPP_OK;
	MppPacket packet = NULL;

    memcpy(ctx->buf_ptr, in_buffer->data, in_buffer->length);

  	ret = ctx->mpi->encode_put_frame(ctx->ctx, ctx->frame);
	if (ret)
	{
		ERR("mpp encode put frame failed");
        return NULL;
	}

	ret = ctx->mpi->encode_get_packet(ctx->ctx, &packet);
	if (ret)
	{
		ERR("mpp encode get packet failed");
        return NULL;
	}

	if (!packet)
	{
		ERR("!packet");
        return NULL;
    }

    void *ptr   = mpp_packet_get_pos(packet);
    size_t len  = mpp_packet_get_length(packet);
    if (len <= 0|| ptr == NULL)
    {
        ERR("encode len error!!!");
        return NULL;
    }

    Buffer *buffer = findFrameNaluBuffer((uint8_t *)ptr, len);
    mpp_packet_deinit(&packet);
    if (!buffer)
        return NULL;
    return buffer;
}

Buffer *getPpsAndSps(MppContext * ctx, int type)
{
    MPP_RET ret = MPP_OK;
    MppPacket packet = NULL;
    mpp_packet_init_with_buffer(&packet, ctx->frm_buf); 
    mpp_packet_set_length(packet, 0);

    ret = ctx->mpi->control(ctx->ctx, MPP_ENC_GET_HDR_SYNC, packet);
    if (ret) {
        ERR("mpi control enc get extra info failed");
        return NULL;
    } 
           
    void *ptr   = mpp_packet_get_pos(packet);
    size_t len  = mpp_packet_get_length(packet);
    if (len <= 0 || ptr == NULL)
    {
        ERR("encode len error!!!");
        return NULL;
    }

    Buffer *buffer = findTypeNaluBuffer((uint8_t *)ptr, len, type);
    mpp_packet_deinit(&packet);
    if (!buffer)
        return NULL;
    return buffer;
}

