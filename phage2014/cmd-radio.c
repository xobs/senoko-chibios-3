
#include "ch.h"
#include "hal.h"

#include <stdlib.h>
#include "chprintf.h"

#include "cmd-radio.h"
#include "phage.h"

/* PA0 */
void radioAddressMatch(EXTDriver *extp, expchannel_t channel)
{
  (void)extp;
  (void)channel;
}

/* PA11 */
void radioDataReceived(EXTDriver *extp, expchannel_t channel)
{
  (void)extp;
  (void)channel;
}

/* PA12 */
void radioCarrier(EXTDriver *extp, expchannel_t channel)
{
  (void)extp;
  (void)channel;
}

static int radioUseCount = 0;
static void radioSelect(void)
{
  /* XXX USE LOCK HERE XXX */
  if (!radioUseCount)
    spiSelect(radio);
  radioUseCount++;
}

static void radioUnselect(void)
{
  /* XXX USE LOCK HERE XXX */
  radioUseCount--;
  if (radioUseCount <= 0) {
    spiUnselect(radio);
    radioUseCount = 0;
  }
}

void radioSetChannel(uint32_t channel)
{
  uint8_t buffer[5];

  radioSelect();
  buffer[0] = 0x22;
  buffer[1] = ((uint8_t *)&channel)[0];
  buffer[2] = ((uint8_t *)&channel)[1];
  buffer[3] = ((uint8_t *)&channel)[2];
  buffer[4] = ((uint8_t *)&channel)[3];

  spiSend(radio, sizeof(buffer), buffer);

  radioUnselect();
}

/*
 * SPI1 configuration structure.
 * Speed 32/16 MHz, CPHA=1, CPOL=1, 8bits frames, MSb transmitted first.
 * The slave select line is the pin GPIOE_CS_SPI on the port GPIOE.
 */
static const SPIConfig spiConfig = {
  NULL,
  /* HW dependent part.*/
  GPIOA,
  PA4,
  SPI_CR1_BR_0 | SPI_CR1_BR_1
};

void radioStart(void)
{
  spiStart(radio, &spiConfig);
}

static uint32_t addr = 0;
void cmd_radio(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint8_t byte;
  uint8_t dat[11];
  uint32_t currAddr;

  (void)argv;

  chprintf(chp, "argc: %d\r\n", argc);
  if (argc == 1) {
//    addr = strtoul(argv[0], NULL, 0);
    chprintf(chp, "Updating channel ID to 0x%08x...\r\n", ++addr);
    radioSetChannel(addr);
  }

  chprintf(chp, "Status value: ");
  byte = 0x10;
  radioSelect();
  spiSend(radio, 1, &byte);
  spiReceive(radio, sizeof(dat), dat);
  chprintf(chp, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
      dat[0], dat[1], dat[2], dat[3], dat[4], dat[5], dat[6], dat[7], dat[8], dat[9], dat[10]);
  radioUnselect();


  radioSelect();
  chprintf(chp, "ID value: ");
  byte = 0x23;
  spiSend(radio, 1, &byte);
  spiReceive(radio, sizeof(currAddr), &currAddr);

  chprintf(chp, "0x%08x\r\n", currAddr);
  radioUnselect();

  return;
}
