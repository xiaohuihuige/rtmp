#ifndef __TYPE_H__
#define __TYPE_H__

#include "chunk_header.h"

#define RTMP_VERSION 			3
#define RTMP_HANDSHAKE_SIZE	 	1536
#define RTMP_OUTPUT_CHUNK_SIZE 	4096

#define RTMP_FMSVER				"FMS/3,0,1,123"
#define RTMP_CAPABILITIES		31

#define RTMP_STREAM_LIVE		"live"
#define RTMP_STREAM_RECORD		"record"
#define RTMP_STREAM_APPEND		"append"

#define RTMP_LEVEL_WARNING		"warning"
#define RTMP_LEVEL_STATUS		"status"
#define RTMP_LEVEL_ERROR		"error"
#define RTMP_LEVEL_FINISH		"finish" // ksyun cdn
#define RTMP_WINDOW_SIZE    	5000000

#define RTMP_FRAME_HEADER_LENGTH 9
#define RTMP_AVC_HEADER_LENGTH   16

#define WIDTH_1280 			1280
#define Height_720 			720
#define DISPLAY_WIDTH_1280 	1280
#define DISPLAY_Height_720 720
#define DURATION 				0
#define FRAMERATE 30
#define FPS 30
#define VIDEODATARATE 0
#define VIDEOCODECID 7
#define VIDEOCODECID_H263 2
#define VIDEOCODECID_H264 7
#define VIDEOCODECID_H265 12

#define AUDIODATARATE 125
#define AUDIOCODECID 10
#define PROFILE 0
#define LEVEL 0

#define AUDIO_CHANNL 1
#define VIDEO_CHANNL 0

#define FILE_MEDIA  0
#define LOCAL_MEDIA 1


enum {
    NAL_UNIT_TYPE_UNSPECIFIED = 0,                    // Unspecified
    NAL_UNIT_TYPE_CODED_SLICE_NON_IDR,                // Coded slice of a non-IDR picture
    NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A,       // Coded slice data partition A
    NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B,       // Coded slice data partition B
    NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C,       // Coded slice data partition C
    NAL_UNIT_TYPE_CODED_SLICE_IDR = 5,                // Coded slice of an IDR picture
    NAL_UNIT_TYPE_SEI,                                // Supplemental enhancement information (SEI)
    NAL_UNIT_TYPE_SPS = 7,                            // Sequence parameter set
    NAL_UNIT_TYPE_PPS = 8,                            // Picture parameter set
    NAL_UNIT_TYPE_AUD,                                // Access unit delimiter
    NAL_UNIT_TYPE_END_OF_SEQUENCE,                    // End of sequence
    NAL_UNIT_TYPE_END_OF_STREAM,                      // End of stream
    NAL_UNIT_TYPE_FILLER,                             // Filler data
    NAL_UNIT_TYPE_SPS_EXT,                            // Sequence parameter set extension
    NAL_UNIT_TYPE_PREFIX_NAL,                         // Prefix NAL unit
    NAL_UNIT_TYPE_SUBSET_SPS,                         // Subset Sequence parameter set
    NAL_UNIT_TYPE_DPS,                                // Depth Parameter Set
                                                      // 17..18    // Reserved
    NAL_UNIT_TYPE_CODED_SLICE_AUX = 19,               // Coded slice of an auxiliary coded picture without partitioning
    NAL_UNIT_TYPE_CODED_SLICE_SVC_EXTENSION,          // Coded slice of SVC extension
                                                      // 20..23    // Reserved
                                                      // 24..31    // Unspecified
};

enum rtmp_chunk_type_t
{
	RTMP_CHUNK_TYPE_0 = 0, // 11-bytes: timestamp(3) + length(3) + stream type(1) + stream id(4)
	RTMP_CHUNK_TYPE_1 = 1, // 7-bytes: delta(3) + length(3) + stream type(1)
	RTMP_CHUNK_TYPE_2 = 2, // 3-bytes: delta(3)
	RTMP_CHUNK_TYPE_3 = 3, // 0-byte
};

typedef enum
{
	RTMP_HANDSHAKE_UNINIT, // Uninitialized 
	RTMP_HANDSHAKE_0, // received C0/S0, wait C1/S1
	RTMP_HANDSHAKE_1, // received C1/S1, wait C2/S2
	RTMP_HANDSHAKE_2, // received C2/S2, handshake done
} Handshake;

enum RTMPMessageTypeId
{
	/* Protocol Control Messages */
	RTMP_TYPE_SET_CHUNK_SIZE = 1,
	RTMP_TYPE_ABORT = 2,
	RTMP_TYPE_ACKNOWLEDGEMENT = 3,			   // bytes read report
	RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE = 5, // server bandwidth
	RTMP_TYPE_SET_PEER_BANDWIDTH = 6,		   // client bandwidth

	/* User Control Messages Event (4) */
	RTMP_TYPE_EVENT = 4,

	RTMP_TYPE_AUDIO = 8,
	RTMP_TYPE_VIDEO = 9,

	/* Data Message */
	RTMP_TYPE_FLEX_STREAM = 15, // AMF3
	RTMP_TYPE_DATA = 18,		// AMF0

	/* Shared Object Message */
	RTMP_TYPE_FLEX_OBJECT = 16,	  // AMF3
	RTMP_TYPE_SHARED_OBJECT = 19, // AMF0

	/* Command Message */
	RTMP_TYPE_FLEX_MESSAGE = 17, // AMF3
	RTMP_TYPE_INVOKE = 20,		 // AMF0

	/* Aggregate Message */
	RTMP_TYPE_METADATA = 22,
};

enum
{
	RTMP_EVENT_STREAM_BEGIN			= 0,
	RTMP_EVENT_STREAM_EOF			= 1,
	RTMP_EVENT_STREAM_DRY			= 2,
	RTMP_EVENT_SET_BUFFER_LENGTH	= 3,
	RTMP_EVENT_STREAM_IS_RECORD		= 4,

	RTMP_EVENT_PING					= 6, // RTMP_EVENT_PING_REQUEST
	RTMP_EVENT_PONG					= 7, // RTMP_EVENT_PING_RESPONSE

	// https://www.gnu.org/software/gnash/manual/doxygen/namespacegnash_1_1rtmp.html
	RTMP_EVENT_REQUEST_VERIFY		= 0x1a,
	RTMP_EVENT_RESPOND_VERIFY		= 0x1b,
	RTMP_EVENT_BUFFER_EMPTY			= 0x1f,
	RTMP_EVENT_BUFFER_READY			= 0x20,
};

enum
{
	RTMP_AMF_EVENT_USE				= 1,
	RTMP_AMF_EVENT_RELEASE			= 2,
	RTMP_AMF_EVENT_REQUEST_CHANGE	= 3,
	RTMP_AMF_EVENT_CHANGE			= 4,
	RTMP_AMF_EVENT_SUCCESS			= 5,
	RTMP_AMF_EVENT_SEND_MESSAGE		= 6,
	RTMP_AMF_EVENT_STATUS			= 7,
	RTMP_AMF_EVENT_CLEAR			= 8,
	RTMP_AMF_EVENT_REMOVE			= 9,
	RTMP_AMF_EVENT_REQUEST_REMOVE	= 10,
	RTMP_AMF_EVENT_USE_SUCCESS		= 11,
};

enum rtmp_channel_t
{
	RTMP_CHANNEL_PROTOCOL = 2, // Protocol Control Messages (1,2,3,5,6) & User Control Messages Event (4)
	RTMP_CHANNEL_INVOKE,	   // RTMP_TYPE_INVOKE (20) & RTMP_TYPE_FLEX_MESSAGE (17)
	RTMP_CHANNEL_AUDIO,		   // RTMP_TYPE_AUDIO (8)
	RTMP_CHANNEL_VIDEO,		   // RTMP_TYPE_VIDEO (9)
	RTMP_CHANNEL_DATA,		   // RTMP_TYPE_DATA (18) & RTMP_TYPE_FLEX_STREAM (15)
	RTMP_CHANNEL_DATA1,		   // RTMP_TYPE_DATA (18) & RTMP_TYPE_FLEX_STREAM (15)
	RTMP_CHANNEL_MAX = 65599,  // The protocol supports up to 65597 streams with IDs 3-65599(65535 + 64)
};

enum {
    StreamBegin,
    StreamEOF,
    StreamDry,
    SetBufferLength,
    StreamIsRecorded,
    PingRequest,
    PingResponse,
};

enum
{
    RTMP_BANDWIDTH_LIMIT_HARD = 0,
    RTMP_BANDWIDTH_LIMIT_SOFT = 1,
    RTMP_BANDWIDTH_LIMIT_DYNAMIC = 2,
};

typedef struct 
{
	int index;
	HeaderChunk header;
	Buffer *buffer;
} RtmpPacket;

typedef struct 
{
	char app[128]; // Server application name, e.g.: testapp
	char flashver[64]; // Flash Player version, FMSc/1.0
	char type[128];
	char swfUrl[256]; // URL of the source SWF file
	char tcUrl[256]; // URL of the Server, rtmp://host:1935/testapp/instance1
	uint8_t fpad; // boolean: True if proxy is being used.
	double capabilities; // double default: 15
	double audioCodecs; // double default: 4071
	double videoCodecs; // double default: 252
	double videoFunction; // double default: 1
	double encoding;
	char pageUrl[256]; // http://host/sample.html
} SessionConfig;

typedef struct
{
    uint32_t in_chunk_size;    // read from network
    uint32_t out_chunk_size;   // write to network
    uint32_t sequence_number;  // bytes read report
    uint32_t window_size;      // server bandwidth (2500000)
    uint32_t peer_bandwidth;   // client bandwidth
    uint32_t buffer_length_ms; // s -> c
    uint8_t  limit_type;       // client bandwidth limit
    uint8_t  receive_audio;    // client don't want receive audio
    uint8_t  receive_video;
} TransmissionConfig;

typedef struct 
{
	Buffer *avc_sequence;
	FifoQueue *queue;
	Buffer *pps_buffer;
    Buffer *sps_buffer;
	int fps;
	int width;
	int height;
	int display_width;
	int display_height;
	int duration;
	int profile_idc;
	int level_idc;
	int frame_count;
	int videodatarate;
	int videocodecid;
	double fractional_part;
} VideoMedia;

typedef struct 
{
	Buffer *adts_sequence;
	FifoQueue *queue;
	int duration;
	int frame_count;
	int stereo;
	int audiocodecid;
	int audiodatarate;
	int audiosamplerate;
	int audiosamplesize;
	double fractional_part;
} AudioMedia;

typedef struct 
{
    char app[64];
    VideoMedia *video;
    AudioMedia *audio;

	VideoMedia *(*createH264Stream)(void *args);
	void (*destroyH264Stream)(VideoMedia *meida);
	Buffer *(*getH264Stream)(VideoMedia *media,int index);

	AudioMedia *(*createAacStream)(void *args);
	void (*destroyAacStream)(AudioMedia *meida);
	Buffer *(*getAacStream)(AudioMedia *media,int index);
} RtmpMedia;

typedef struct 
{
	int time_base;
   int index;
} MediaChannle;

typedef struct 
{
	int type;
    const char *app;
    const char *h264_file;
    const char *aac_file;
} RtmpConfig;

#endif