#ifndef __RADIO_H__
#define __RADIO_H__

#include "ext.h"
#include "shell.h"

#define radio (&SPID1)

void radioAddressMatch(EXTDriver *extp, expchannel_t channel);
void radioDataReceived(EXTDriver *extp, expchannel_t channel);
void radioCarrier(EXTDriver *extp, expchannel_t channel);

void cmdRadio(BaseSequentialStream *chp, int argc, char *argv[]);

void radioStart(void);

#endif /* __RADIO_H__ */
