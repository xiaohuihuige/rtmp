#include "h264.h"

void read_hrd_parameters(hrd_t *hrd, bs_t *b)
{
    hrd->cpb_cnt_minus1 = bs_read_ue(b);
    hrd->bit_rate_scale = bs_read_u(b, 4);
    hrd->cpb_size_scale = bs_read_u(b, 4);
    for (int SchedSelIdx = 0; SchedSelIdx <= hrd->cpb_cnt_minus1; SchedSelIdx++)
    {
        hrd->bit_rate_value_minus1[SchedSelIdx] = bs_read_ue(b);
        hrd->cpb_size_value_minus1[SchedSelIdx] = bs_read_ue(b);
        hrd->cbr_flag[SchedSelIdx] = bs_read_u1(b);
    }
    hrd->initial_cpb_removal_delay_length_minus1 = bs_read_u(b, 5);
    hrd->cpb_removal_delay_length_minus1 = bs_read_u(b, 5);
    hrd->dpb_output_delay_length_minus1 = bs_read_u(b, 5);
    hrd->time_offset_length = bs_read_u(b, 5);
}

void read_scaling_list(bs_t *b, int *scalingList, int sizeOfScalingList, int *useDefaultScalingMatrixFlag)
{
    // NOTE need to be able to set useDefaultScalingMatrixFlag when reading, hence passing as pointer
    int lastScale = 8;
    int nextScale = 8;
    int delta_scale;
    for (int j = 0; j < sizeOfScalingList; j++)
    {
        if (nextScale != 0)
        {
            if (0)
            {
                nextScale = scalingList[j];
                if (useDefaultScalingMatrixFlag[0])
                {
                    nextScale = 0;
                }
                delta_scale = (nextScale - lastScale) % 256;
            }

            delta_scale = bs_read_se(b);

            if (1)
            {
                nextScale = (lastScale + delta_scale + 256) % 256;
                useDefaultScalingMatrixFlag[0] = (j == 0 && nextScale == 0);
            }
        }
        if (1)
        {
            scalingList[j] = (nextScale == 0) ? lastScale : nextScale;
        }
        lastScale = scalingList[j];
    }
}

#define SAR_Extended 255 // Extended_SAR
void read_vui_parameters(sps_t *sps, bs_t *b)
{
    sps->vui.aspect_ratio_info_present_flag = bs_read_u1(b);
    if (sps->vui.aspect_ratio_info_present_flag)
    {
        sps->vui.aspect_ratio_idc = bs_read_u8(b);
        if (sps->vui.aspect_ratio_idc == SAR_Extended)
        {
            sps->vui.sar_width = bs_read_u(b, 16);
            sps->vui.sar_height = bs_read_u(b, 16);
        }
    }
    sps->vui.overscan_info_present_flag = bs_read_u1(b);
    if (sps->vui.overscan_info_present_flag)
    {
        sps->vui.overscan_appropriate_flag = bs_read_u1(b);
    }
    sps->vui.video_signal_type_present_flag = bs_read_u1(b);
    if (sps->vui.video_signal_type_present_flag)
    {
        sps->vui.video_format = bs_read_u(b, 3);
        sps->vui.video_full_range_flag = bs_read_u1(b);
        sps->vui.colour_description_present_flag = bs_read_u1(b);
        if (sps->vui.colour_description_present_flag)
        {
            sps->vui.colour_primaries = bs_read_u8(b);
            sps->vui.transfer_characteristics = bs_read_u8(b);
            sps->vui.matrix_coefficients = bs_read_u8(b);
        }
    }
    sps->vui.chroma_loc_info_present_flag = bs_read_u1(b);
    if (sps->vui.chroma_loc_info_present_flag)
    {
        sps->vui.chroma_sample_loc_type_top_field = bs_read_ue(b);
        sps->vui.chroma_sample_loc_type_bottom_field = bs_read_ue(b);
    }
    sps->vui.timing_info_present_flag = bs_read_u1(b);
    if (sps->vui.timing_info_present_flag)
    {
        sps->vui.num_units_in_tick = bs_read_u(b, 32);
        sps->vui.time_scale = bs_read_u(b, 32);
        sps->vui.fixed_frame_rate_flag = bs_read_u1(b);
    }
    sps->vui.nal_hrd_parameters_present_flag = bs_read_u1(b);
    if (sps->vui.nal_hrd_parameters_present_flag)
    {
        read_hrd_parameters(&sps->hrd_nal, b);
    }
    sps->vui.vcl_hrd_parameters_present_flag = bs_read_u1(b);
    if (sps->vui.vcl_hrd_parameters_present_flag)
    {
        read_hrd_parameters(&sps->hrd_vcl, b);
    }
    if (sps->vui.nal_hrd_parameters_present_flag || sps->vui.vcl_hrd_parameters_present_flag)
    {
        sps->vui.low_delay_hrd_flag = bs_read_u1(b);
    }
    sps->vui.pic_struct_present_flag = bs_read_u1(b);
    sps->vui.bitstream_restriction_flag = bs_read_u1(b);
    if (sps->vui.bitstream_restriction_flag)
    {
        sps->vui.motion_vectors_over_pic_boundaries_flag = bs_read_u1(b);
        sps->vui.max_bytes_per_pic_denom = bs_read_ue(b);
        sps->vui.max_bits_per_mb_denom = bs_read_ue(b);
        sps->vui.log2_max_mv_length_horizontal = bs_read_ue(b);
        sps->vui.log2_max_mv_length_vertical = bs_read_ue(b);
        sps->vui.num_reorder_frames = bs_read_ue(b);
        sps->vui.max_dec_frame_buffering = bs_read_ue(b);
    }
}

void read_seq_parameter_set_rbsp(sps_t *sps, bs_t *b)
{
    int i;

    if (1)
    {
        memset(sps, 0, sizeof(sps_t));
        sps->chroma_format_idc = 1;
    }
    bs_read_u8(b);
    sps->profile_idc = bs_read_u8(b);
    ERR("%d", sps->profile_idc);
    sps->constraint_set0_flag = bs_read_u1(b);
    sps->constraint_set1_flag = bs_read_u1(b);
    sps->constraint_set2_flag = bs_read_u1(b);
    sps->constraint_set3_flag = bs_read_u1(b);
    sps->constraint_set4_flag = bs_read_u1(b);
    sps->constraint_set5_flag = bs_read_u1(b);
    /* reserved_zero_2bits */ bs_skip_u(b, 2);
    sps->level_idc = bs_read_u8(b);
    sps->seq_parameter_set_id = bs_read_ue(b);

    if (sps->profile_idc == 100 || sps->profile_idc == 110 ||
        sps->profile_idc == 122 || sps->profile_idc == 244 ||
        sps->profile_idc == 44 || sps->profile_idc == 83 ||
        sps->profile_idc == 86 || sps->profile_idc == 118 ||
        sps->profile_idc == 128 || sps->profile_idc == 138 ||
        sps->profile_idc == 139 || sps->profile_idc == 134 || sps->profile_idc == 103)
    {
        sps->chroma_format_idc = bs_read_ue(b);
        if (sps->chroma_format_idc == 3)
        {
            sps->residual_colour_transform_flag = bs_read_u1(b);
        }
        sps->bit_depth_luma_minus8 = bs_read_ue(b);
        sps->bit_depth_chroma_minus8 = bs_read_ue(b);
        sps->qpprime_y_zero_transform_bypass_flag = bs_read_u1(b);
        sps->seq_scaling_matrix_present_flag = bs_read_u1(b);
        if (sps->seq_scaling_matrix_present_flag)
        {
            for (i = 0; i < ((sps->chroma_format_idc != 3) ? 8 : 12); i++)
            {
                sps->seq_scaling_list_present_flag[i] = bs_read_u1(b);
                if (sps->seq_scaling_list_present_flag[i])
                {
                    if (i < 6)
                    {
                        read_scaling_list(b, sps->ScalingList4x4[i], 16,
                                          &(sps->UseDefaultScalingMatrix4x4Flag[i]));
                    }
                    else
                    {
                        read_scaling_list(b, sps->ScalingList8x8[i - 6], 64,
                                          &(sps->UseDefaultScalingMatrix8x8Flag[i - 6]));
                    }
                }
            }
        }
    }
    sps->log2_max_frame_num_minus4 = bs_read_ue(b);
    sps->pic_order_cnt_type = bs_read_ue(b);
    if (sps->pic_order_cnt_type == 0)
    {
        sps->log2_max_pic_order_cnt_lsb_minus4 = bs_read_ue(b);
    }
    else if (sps->pic_order_cnt_type == 1)
    {
        sps->delta_pic_order_always_zero_flag = bs_read_u1(b);
        sps->offset_for_non_ref_pic = bs_read_se(b);
        sps->offset_for_top_to_bottom_field = bs_read_se(b);
        sps->num_ref_frames_in_pic_order_cnt_cycle = bs_read_ue(b);
        for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            sps->offset_for_ref_frame[i] = bs_read_se(b);
        }
    }
    sps->num_ref_frames = bs_read_ue(b);
    sps->gaps_in_frame_num_value_allowed_flag = bs_read_u1(b);
    sps->pic_width_in_mbs_minus1 = bs_read_ue(b);
    sps->pic_height_in_map_units_minus1 = bs_read_ue(b);
    int width = (sps->pic_width_in_mbs_minus1 + 1) * 16;
    int height = (sps->pic_height_in_map_units_minus1 + 1) * 16;
    printf("width %d, height %d", width, height);

    sps->frame_mbs_only_flag = bs_read_u1(b);
    if (!sps->frame_mbs_only_flag)
    {
        sps->mb_adaptive_frame_field_flag = bs_read_u1(b);
    }
    sps->direct_8x8_inference_flag = bs_read_u1(b);
    sps->frame_cropping_flag = bs_read_u1(b);
    if (sps->frame_cropping_flag)
    {
        sps->frame_crop_left_offset = bs_read_ue(b);
        sps->frame_crop_right_offset = bs_read_ue(b);
        sps->frame_crop_top_offset = bs_read_ue(b);
        sps->frame_crop_bottom_offset = bs_read_ue(b);
    }
    sps->vui_parameters_present_flag = bs_read_u1(b);
    if (sps->vui_parameters_present_flag)
    {
        read_vui_parameters(sps, b);
    }
}

int find_nal_unit(uint8_t *buf, int size, int *nal_start, int *nal_end)
{
    int i;
    // find start
    *nal_start = 0;
    *nal_end = 0;

    i = 0;
    while ( //( next_bits( 24 ) != 0x000001 && next_bits( 32 ) != 0x00000001 )
        (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01) &&
        (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0 || buf[i + 3] != 0x01))
    {
        i++; // skip leading zero
        if (i + 4 >= size)
        {
            return 0;
        } // did not find nal start
    }

    if (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01) // ( next_bits( 24 ) != 0x000001 )
    {
        i++;
    }

    if (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01)
    { /* error, should never happen */
        return 0;
    }
    i += 3;
    *nal_start = i;

    while ( //( next_bits( 24 ) != 0x000000 && next_bits( 24 ) != 0x000001 )
        (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0) &&
        (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01))
    {
        i++;
        // FIXME the next line fails when reading a nal that ends exactly at the end of the data
        if (i + 3 >= size)
        {
            *nal_end = size;
            return -1;
        } // did not find nal end, stream ended first
    }

    *nal_end = i;
    return (*nal_end - *nal_start);
}