// #include "rtmp_media.h"

// RtmpMedia *createRtmpMedia(const char *app_name, const char *h264_filepath, const char *aac_filepath)
// {
//     RtmpMedia *media = CALLOC(1, RtmpMedia);
//     if (!media)
//         return NULL;


//     if (h264_filepath) {
//         media->video = CALLOC(1, VideoMedia);
//         INIT_LIST_HEAD(&media->video->cache.list);
//     }


//     if (aac_filepath) {
//         media->audio = CALLOC(1, VideoMedia);
//         INIT_LIST_HEAD(&media->video->cache.list);
//     }
// }