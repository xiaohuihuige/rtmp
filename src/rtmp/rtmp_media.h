// #ifndef __RTMP_MEDIA_H__
// #define __RTMP_MEDIA_H__

// #include <schedule/net-common.h>
// #include <schedule/tcp_server.h>
// #include <schedule/timestamp.h>
// #include "type.h"

// typedef struct
// {
//     Buffer *buffer;
//     struct list_head list;
// } GopCache;

// typedef struct 
// {
//     GopCache cache;
// } AudioMedia;

// typedef struct 
// {
//     int fps;
//     int width; 
//     int height;
//     GopCache cache;
// } VideoMedia;

// typedef struct 
// {
//     char app_name[64];
//     AudioMedia *audio;
//     VideoMedia *video;
// } RtmpMedia;

// RtmpMedia *createRtmpMedia(const char *app_name, const char *h264_filepath, const char *aac_filepath);

// #endif