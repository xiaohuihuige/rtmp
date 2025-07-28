#ifndef __ALSA_READ_H__
#define __ALSA_READ_H__

#include <schedule/net-common.h>
#include "type.h"

AudioMedia *createAlsaAacMedia(const char *file);
Buffer *getAlsaAacMediaFrame(AudioMedia *media);
void destroyAlsaAacMedia(AudioMedia *media);

#endif