#ifndef __AAC_H__
#define __AAC_H__

#include <schedule/net-common.h>
#include "type.h"

AudioMedia *createAacMedia(const char *file);
void destroyAacMedia(AudioMedia *media);
Buffer *getAacMediaFrame(AudioMedia *media);

#endif //  __AAC_H__
