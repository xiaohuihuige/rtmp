#ifndef __H264_H__
#define __H264_H__

#include <schedule/net-common.h>

enum
{
    NAL_UNIT_TYPE_UNSPECIFIED = 0,              // Unspecified
    NAL_UNIT_TYPE_CODED_SLICE_NON_IDR,          // Coded slice of a non-IDR picture
    NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A, // Coded slice data partition A
    NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B, // Coded slice data partition B
    NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C, // Coded slice data partition C
    NAL_UNIT_TYPE_CODED_SLICE_IDR = 5,          // Coded slice of an IDR picture
    NAL_UNIT_TYPE_SEI,                          // Supplemental enhancement information (SEI)
    NAL_UNIT_TYPE_SPS = 7,                      // Sequence parameter set
    NAL_UNIT_TYPE_PPS = 8,                      // Picture parameter set
    NAL_UNIT_TYPE_AUD,                          // Access unit delimiter
    NAL_UNIT_TYPE_END_OF_SEQUENCE,              // End of sequence
    NAL_UNIT_TYPE_END_OF_STREAM,                // End of stream
    NAL_UNIT_TYPE_FILLER,                       // Filler data
    NAL_UNIT_TYPE_SPS_EXT,                      // Sequence parameter set extension
    NAL_UNIT_TYPE_PREFIX_NAL,                   // Prefix NAL unit
    NAL_UNIT_TYPE_SUBSET_SPS,                   // Subset Sequence parameter set
    NAL_UNIT_TYPE_DPS,                          // Depth Parameter Set
                                                // 17..18    // Reserved
    NAL_UNIT_TYPE_CODED_SLICE_AUX = 19,         // Coded slice of an auxiliary coded picture without partitioning
    NAL_UNIT_TYPE_CODED_SLICE_SVC_EXTENSION,    // Coded slice of SVC extension
                                                // 20..23    // Reserved
                                                // 24..31    // Unspecified
};

typedef struct
{
    int cpb_cnt_minus1;
    int bit_rate_scale;
    int cpb_size_scale;
    int bit_rate_value_minus1[32]; // up to cpb_cnt_minus1, which is <= 31
    int cpb_size_value_minus1[32];
    int cbr_flag[32];
    int initial_cpb_removal_delay_length_minus1;
    int cpb_removal_delay_length_minus1;
    int dpb_output_delay_length_minus1;
    int time_offset_length;
} hrd_t;

typedef struct
{
    int profile_idc;
    int constraint_set0_flag;
    int constraint_set1_flag;
    int constraint_set2_flag;
    int constraint_set3_flag;
    int constraint_set4_flag;
    int constraint_set5_flag;
    int reserved_zero_2bits;
    int level_idc;
    int seq_parameter_set_id;
    int chroma_format_idc;
    int residual_colour_transform_flag;
    int bit_depth_luma_minus8;
    int bit_depth_chroma_minus8;
    int qpprime_y_zero_transform_bypass_flag;
    int seq_scaling_matrix_present_flag;
    int seq_scaling_list_present_flag[12];
    int ScalingList4x4[6][16];
    int UseDefaultScalingMatrix4x4Flag[6];
    int ScalingList8x8[6][64];
    int UseDefaultScalingMatrix8x8Flag[6];
    int log2_max_frame_num_minus4;
    int pic_order_cnt_type;
    int log2_max_pic_order_cnt_lsb_minus4;
    int delta_pic_order_always_zero_flag;
    int offset_for_non_ref_pic;
    int offset_for_top_to_bottom_field;
    int num_ref_frames_in_pic_order_cnt_cycle;
    int offset_for_ref_frame[256];
    int num_ref_frames;
    int gaps_in_frame_num_value_allowed_flag;
    int pic_width_in_mbs_minus1;
    int pic_height_in_map_units_minus1;
    int frame_mbs_only_flag;
    int mb_adaptive_frame_field_flag;
    int direct_8x8_inference_flag;
    int frame_cropping_flag;
    int frame_crop_left_offset;
    int frame_crop_right_offset;
    int frame_crop_top_offset;
    int frame_crop_bottom_offset;
    int vui_parameters_present_flag;

    struct
    {
        int aspect_ratio_info_present_flag;
        int aspect_ratio_idc;
        int sar_width;
        int sar_height;
        int overscan_info_present_flag;
        int overscan_appropriate_flag;
        int video_signal_type_present_flag;
        int video_format;
        int video_full_range_flag;
        int colour_description_present_flag;
        int colour_primaries;
        int transfer_characteristics;
        int matrix_coefficients;
        int chroma_loc_info_present_flag;
        int chroma_sample_loc_type_top_field;
        int chroma_sample_loc_type_bottom_field;
        int timing_info_present_flag;
        int num_units_in_tick;
        int time_scale;
        int fixed_frame_rate_flag;
        int nal_hrd_parameters_present_flag;
        int vcl_hrd_parameters_present_flag;
        int low_delay_hrd_flag;
        int pic_struct_present_flag;
        int bitstream_restriction_flag;
        int motion_vectors_over_pic_boundaries_flag;
        int max_bytes_per_pic_denom;
        int max_bits_per_mb_denom;
        int log2_max_mv_length_horizontal;
        int log2_max_mv_length_vertical;
        int num_reorder_frames;
        int max_dec_frame_buffering;
    } vui;

    hrd_t hrd_nal;
    hrd_t hrd_vcl;

} sps_t;

int find_nal_unit(uint8_t *buf, int size, int *nal_start, int *nal_end);

#endif // !__H264_H__
