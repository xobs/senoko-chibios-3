
#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include <stdlib.h>

#include "radio.h"
#include "phage2014.h"

/* PA0 */
void radioAddressMatch(EXTDriver *extp, expchannel_t channel)
{
  (void)extp;
  (void)channel;
  chprintf(stream, "RADIO: Detected address match\r\n");
}

/* PA11 */
void radioDataReceived(EXTDriver *extp, expchannel_t channel)
{
  (void)extp;
  (void)channel;
  chprintf(stream, "RADIO: Received data\r\n");
}

/* PA12 */
void radioCarrier(EXTDriver *extp, expchannel_t channel)
{
  (void)extp;
  (void)channel;
  chprintf(stream, "RADIO: Detected carrier\r\n");
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
  uint8_t byte;
  uint8_t dat[11];
  uint32_t currAddr;

  spiStart(radio, &spiConfig);

  palWritePad(GPIOB, PB12, PAL_HIGH); // power on the radio

  // Set frequency to 905 MHz = ((905/2) - 422.4) * 10 = 301 = 0x12D    // might need to change to be backward-compatible
  // hfreq_pll = 1
  // pa_pwr = 11 +10 dBm
  // RxRed_pw = 0 
  // auto_retran = 0
  // rx_afw = 100  (4-byte RX address)
  // tx_afw = 100  (4-byte TX address)
  // RX_PW = 1  (1 byte payload)
  // TX_PW = 1  (1 byte payload)
  // RX_address = 0x034DBABE
  // UP_CLK_FREQ = 11 (500 KHz, not used)
  // UP_CLK_EN = 0  (no clock)
  // XOF = 011, 16 MHz
  // CRC_EN = 1 enable
  // CRC_MODE = 0  / 8-bit check
  
  // payload values:
  // 0: 0x2D
  // 1: 0x0F   XX 0 0 11 1 1
  // 2: 0x44
  // 3: 0x01
  // 4: 0x01
  // 5: 0x03
  // 6: 0x4D
  // 7: 0xBA
  // 8: 0xBE
  // 9: 0x5B   0 1 011 0 11    0101 1011 
  dat[0] = 0x00;  // write command to CONFIG from address 0
  dat[1] = 0x2D;  // might need changing to be backward-compatible with old radios
  dat[2] = 0x0F;
  dat[3] = 0x44;
  dat[4] = 0x01;
  dat[5] = 0x01;
  dat[6] = 0x03;
  dat[7] = 0x4D;
  dat[8] = 0x4D;
  dat[9] = 0x03;
  dat[10] = 0x5B;
  
  radioSelect();
  spiSend(radio, 11, dat);
  radioUnselect();

  dat[0] = 0x22;   // write TX address 
  dat[1] = 0x03;
  dat[2] = 0x4D;
  dat[3] = 0x4D;
  dat[4] = 0x03;
  radioSelect();
  spiSend(radio, 5, dat);
  radioUnselect();

  palWritePad(GPIOB, PB14, PAL_LOW); // set to receive mode (TX_EN low)
  palWritePad(GPIOB, PB13, PAL_HIGH); // set TRXCE to high


}

void radio_TX(uint8_t data) {
  uint8_t dat[2];
  
  palWritePad(GPIOB, PB14, PAL_HIGH); // set to tx mode (TX_EN high)

  dat[0] = 0x20; // write TX payload
  dat[1] = data; 
  radioSelect();
  spiSend(radio, 2, dat);
  radioUnselect();

  palWritePad(GPIOB, PB13, PAL_HIGH); // set TRXCE to high
  chThdSleepMicroseconds(15);
  palWritePad(GPIOB, PB13, PAL_LOW); // set TRXCE to low

  chThdSleepMicroseconds(800);

  palWritePad(GPIOB, PB14, PAL_LOW); // set to tx mode (TX_EN high)
  palWritePad(GPIOB, PB13, PAL_HIGH); // set TRXCE to high (to allow for receiving)

}

uint8_t radio_RX(void) {
  uint8_t cmd;
  uint8_t dat[2];
  
  cmd = 0x24;
  radioSelect();
  spiSend(radio, 1, &cmd);

  spiReceive(radio, sizeof(dat), &dat);
  radioUnselect();

  chprintf( stream, "%08x %8x\r\n", dat[0], dat[1] );
  return dat[0];

}

static uint32_t addr = 0;
void cmdRadio(BaseSequentialStream *chp, int argc, char *argv[])
{
  uint8_t byte;
  uint8_t dat[11];
  uint32_t currAddr;
  int i;

#if 1
  for( i = 0; i < 0x28; i += 4 ) {
    chprintf(stream, "RCC + %x: %x\r\n", i, *((unsigned int *) (0x40021000 + i)) );
  }
#endif

#if 1  

  chprintf(chp, "argc: %d\r\n", argc);
  if (argc == 1) {
//    addr = strtoul(argv[0], NULL, 0);
//    chprintf(chp, "Updating channel ID to 0x%08x...\r\n", ++addr);
//    radioSetChannel(addr);
  }

  chprintf(chp, "Config value: ");
  byte = 0x10;
  radioSelect();
  spiSend(radio, 1, &byte);
  spiReceive(radio, sizeof(dat), dat);
  chprintf(chp, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
      dat[0], dat[1], dat[2], dat[3], dat[4], dat[5], dat[6], dat[7], dat[8], dat[9], dat[10]);
  radioUnselect();


  radioSelect();
  chprintf(chp, "TX address: ");
  byte = 0x23;
  spiSend(radio, 1, &byte);
  spiReceive(radio, sizeof(currAddr), &currAddr);

  chprintf(chp, "0x%08x\r\n", currAddr);
  radioUnselect();
#endif

  return;
}

