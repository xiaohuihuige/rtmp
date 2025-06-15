#ifndef __UTIL_H__
#define __UTIL_H__

#include <schedule/net-common.h>
#include <schedule/tcp_server.h>
#include <schedule/timestamp.h>

Buffer *readMediaFile(const char *file_path);
void printfRtmpAddr(int port, const char *app);

uint32_t calculateTimeStamp(double *fractional_part, int fps, int sample_number);

#endif