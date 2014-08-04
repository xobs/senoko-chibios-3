#ifndef __PHAGE_RADIO_H__
#define __PHAGE_RADIO_H__

#define radio (&SPID1)

void radioStart(void);
void radioSend(uint8_t byte);
uint32_t radioGetAddress(void);
void radioGetStatus(uint8_t buf[11]);
void radioSetChannel(uint32_t channel);

#endif /* __PHAGE_RADIO_H__ */
