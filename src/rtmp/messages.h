#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "type.h"
#include <schedule/net-common.h>
#include "rtmp_session.h"

typedef struct 
{
    int code;
    char error[126];
} stateMessage;

stateMessage state[] = {
    {200,                                   "result"},
    {RTMP_TYPE_SET_CHUNK_SIZE,              "RTMP_TYPE_SET_CHUNK_SIZE"},
    {RTMP_TYPE_ABORT,                       "RTMP_TYPE_ABORT"},
    {RTMP_TYPE_ACKNOWLEDGEMENT,             "RTMP_TYPE_ACKNOWLEDGEMENT"},
    {RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE, "RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE"},
    {RTMP_TYPE_SET_PEER_BANDWIDTH,          "RTMP_TYPE_SET_PEER_BANDWIDTH"},
    {RTMP_TYPE_EVENT,                       "RTMP_TYPE_EVENT"},
    {RTMP_TYPE_AUDIO,                       "RTMP_TYPE_AUDIO"},
    {RTMP_TYPE_VIDEO,                       "RTMP_TYPE_VIDEO"},
    {RTMP_TYPE_FLEX_STREAM,                 "RTMP_TYPE_FLEX_STREAM"},
    {RTMP_TYPE_DATA,                        "RTMP_TYPE_DATA"},
    {RTMP_TYPE_FLEX_OBJECT,                 "RTMP_TYPE_FLEX_OBJEC"},
    {RTMP_TYPE_SHARED_OBJECT,               "RTMP_TYPE_SHARED_OBJECT"},
    {RTMP_TYPE_FLEX_MESSAGE,                "RTMP_TYPE_FLEX_MESSAGE"},
    {RTMP_TYPE_INVOKE,                      "RTMP_TYPE_INVOKE"},
    {RTMP_TYPE_METADATA,                    "RTMP_TYPE_METADATA"},
};


static inline void printfMessage(char *message, int code)
{
    for (int i = 0; i < sizeof(state)/sizeof(stateMessage); i++) {
        if (code == state[i].code)
            LOG("%s %s", message, state[i].error);
    }
}


#endif // !__MESSAGES_H__
