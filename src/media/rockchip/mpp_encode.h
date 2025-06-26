#ifndef __MPP_ENCODE_H__
#define __MPP_ENCODE_H__

#include <schedule/net-common.h>
#include <schedule/buffer.h>
#include <rockchip/rk_mpi.h>
#include <rockchip/mpp_frame.h>
#include <rockchip/rk_mpi.h>
#include <rockchip/mpp_rc_api.h>

#define MPP_ALIGN(x, a)         (((x)+(a)-1)&~((a)-1))

typedef struct
{
    RK_S32 qp_init;
    RK_S32 qp_max;
    RK_S32 qp_min;
    RK_S32 qp_max_i;
    RK_S32 qp_min_i;
    RK_S32 qp_delta_ip;

    RK_S32 fps_in_flex;
    RK_S32 fps_in_den;
    RK_S32 fps_in_num;
    RK_S32 fps_out_flex;
    RK_S32 fps_out_den;
    RK_S32 fps_out_num;
    RK_S32 bps;
    RK_S32 bps_max;
    RK_S32 bps_min;
    RK_S32 rc_mode;
    RK_S32 gop_mode;
    RK_S32 gop_len;
    RK_S32 vi_len;
    RK_S32 scene_mode;
    RK_S32 deblur_en;
    RK_S32 header_mode;
    MppEncSeiMode sei_mode;

    RK_S32 cu_qp_delta_depth;
    RK_S32 anti_flicker_str;

} EncodeConfig;

typedef struct
{
    MppCtx ctx;
    MppApi *mpi;

    // global flow control flag
    RK_U32 frm_eos;
    RK_U32 pkt_eos;
    RK_U32 frame_count;
    RK_U64 stream_size;

    /* encoder config set */
    MppEncCfg cfg;
    EncodeConfig config;

    // input / output
    MppBuffer frm_buf;

    // paramter for resource malloc
    RK_S32 fps;
    RK_U32 width;
    RK_U32 height;
    RK_U32 hor_stride;
    RK_U32 ver_stride;
    MppFrameFormat fmt;
    MppCodingType type;
    RK_U32 num_frames;
    // resources
    size_t frame_size;
    /* NOTE: packet buffer may overflow */
    size_t packet_size;

    void *buf_ptr;
    MppFrame frame;
} MppContext;

MppContext *createMppEncode(int width, int height, int fps);
void destroyMppEncode(MppContext *ctx);

Buffer *encodeMppFrame(MppContext *ctx, Buffer *in_buffer);
Buffer *getPpsAndSps(MppContext * ctx);

#endif