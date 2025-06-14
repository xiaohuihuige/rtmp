#ifndef __ALSA_READ_H__
#define __ALSA_READ_H__

#include <schedule/net-common.h>
#include "type.h"

AudioMedia *createAlsaAacMedia(const char *device_name);
void destroyAlsaAacMedia(AudioMedia *media);
Buffer *getAlsaAacMediaFrame(AudioMedia *media, int index);

#endif