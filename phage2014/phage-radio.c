
#include "ch.h"
#include "hal.h"

#include <stdlib.h>
#include "chprintf.h"

#include "phage-radio.h"
#include "phage.h"

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

static int radioUseCount = 0;
static void radio_select(void) {
  /* XXX USE LOCK HERE XXX */
  if (!radioUseCount) {
    spiSelect(radio);
  }
  radioUseCount++;
}

static void radio_unselect(void) {
  /* XXX USE LOCK HERE XXX */
  radioUseCount--;
  if (radioUseCount <= 0) {
    spiUnselect(radio);
    radioUseCount = 0;
  }
}

static void radio_spi_start(SPIDriver *spip, const SPIConfig *config) {
  spip->config = config;

  rccEnableSPI1(FALSE);

  /* SPI setup and enable.*/
  spip->spi->CR1  = 0;
  spip->spi->CR1  = spip->config->cr1 | SPI_CR1_MSTR | SPI_CR1_SSM |
                    SPI_CR1_SSI;
  spip->spi->CR2  = SPI_CR2_SSOE;
  spip->spi->CR1 |= SPI_CR1_SPE;

  spip->state = SPI_READY;
}

void radio_lld_send(size_t n, const void *txbuf) {
  const uint8_t *buf = (const uint8_t *)txbuf;
  unsigned int i;

  for (i = 0; i < n; i++) {
    /* Wait for buffer to empty.*/
    while ( (radio->spi->SR & SPI_SR_TXE) == 0);
    radio->spi->DR = buf[i];
  }
  /* Wait for buffer to empty.*/
  while ( (radio->spi->SR & SPI_SR_TXE) == 0);
}

void radio_lld_receive(size_t n, void *rxbuf) {
  uint8_t *buf = (uint8_t *)rxbuf;
  unsigned int i;

  for (i = 0; i < n; i++) {
    /* Write dummy byte.*/
    radio->spi->DR = 0xff;

    /* Wait for buffer to have data.*/
    while ( (radio->spi->SR & SPI_SR_RXNE) == 0 );
    buf[i] = radio->spi->DR;
  }
}

void radio_lld_txrx(size_t n, const void *txbuf, const void *rxbuf) {
  const uint8_t *buf = (const uint8_t *)txbuf;
  uint8_t *rbuf = (uint8_t *)rxbuf;

  unsigned int i, j;
  unsigned char first = 1;

  j = 0;
  rbuf[n-1] = 0xDD; // fill in with some default value
  
  for (i = 0; i < n; i++) {
    /* Wait for buffer to empty.*/
    while ( (radio->spi->SR & SPI_SR_TXE) == 0);
    if( first ) {
      first--; // data before first write completes is garbage
      rbuf[0] = radio->spi->DR; // clear the DR FIFO of any cruft before going forward
    } else {
      if( rxbuf != NULL ) {
	rbuf[j] = radio->spi->DR;
	j++;
      }
    }
    radio->spi->DR = buf[i];

  }
  /* Wait for buffer to empty.*/
  while ( (radio->spi->SR & SPI_SR_TXE) == 0);
  if( rxbuf != NULL ) {
    rbuf[j++] = radio->spi->DR;
  }
  
  // wait for transmission to complete before leaving routine
  chThdSleepMicroseconds(1);

}



void radioSetChannel(uint32_t channel) {
  uint8_t buffer[5];
  uint8_t rdat[5];

  radio_select();

  buffer[0] = 0x22;
  buffer[1] = ((uint8_t *)&channel)[0];
  buffer[2] = ((uint8_t *)&channel)[1];
  buffer[3] = ((uint8_t *)&channel)[2];
  buffer[4] = ((uint8_t *)&channel)[3];

  radio_lld_txrx(sizeof(buffer), buffer, rdat);

  radio_unselect();
}

uint32_t radioGetAddress(void) {
  uint32_t currAddr;
  uint8_t buf[5];
  uint8_t rbuf[5];
  

  radio_select();
  buf[0] = 0x23;
  radio_lld_txrx(5, buf, rbuf);
  radio_unselect();

  currAddr = rbuf[1] | (rbuf[2] << 8) | (rbuf[3] << 16) | (rbuf[4] << 24);

  return currAddr;
}

void radioGetStatus(uint8_t buf[11]) {
  uint8_t tbuf[11];
  tbuf[0] = 0x10;

  radio_select();
  radio_lld_txrx(11, tbuf, buf);
  radio_unselect();
}

void radioGetRxPayload(uint8_t buf[3]) {
  uint8_t tbuf[3];
  tbuf[0] = 0x24;;

  radio_select();
  radio_lld_txrx(3, tbuf, buf);
  radio_unselect();
}

void radioStart(void)
{
  uint8_t dat[11];
  uint8_t rdat[11];

  radio_spi_start(radio, &spiConfig);
  radio_unselect();
  chThdSleepMicroseconds(800);

  /* Power on the radio.*/
  palWritePad(GPIOB, PB12, PAL_HIGH);

  /* Set frequency to 905 MHz = ((905/2) - 422.4) * 10 = 301 = 0x12D
   * might need to change to be backward-compatible
   * hfreq_pll = 1
   * pa_pwr = 11 +10 dBm
   * RxRed_pw = 0
   * auto_retran = 0
   * rx_afw = 100 (4-byte RX address)
   * tx_afw = 100 (4-byte TX address)
   * RX_PW = 1 (1 byte payload)
   * TX_PW = 1 (1 byte payload)
   * RX_address = 0x034DBABE
   * UP_CLK_FREQ = 11 (500 KHz, not used)
   * UP_CLK_EN = 0 (no clock)
   * XOF = 011, 16 MHz
   * CRC_EN = 1 enable
   * CRC_MODE = 0 / 8-bit check

   * payload values:
   * 0: 0x2D
   * 1: 0x0F XX 0 0 11 1 1
   * 2: 0x44
   * 3: 0x01
   * 4: 0x01
   * 5: 0x03
   * 6: 0x4D
   * 7: 0xBA
   * 8: 0xBE
   * 9: 0x5B 0 1 011 0 11 0101 1011
   */

  dat[0] = 0x00; /* Write command to CONFIG from address 0.*/

  /* Might need changing to be backward-compatible with old radios.*/
  dat[1] = 0x2D;
  dat[2] = 0x0F;
  dat[3] = 0x44;
  dat[4] = 0x01;
  dat[5] = 0x01;
  dat[6] = 0x03;
  dat[7] = 0x4D;
  dat[8] = 0x4D;
  dat[9] = 0x03;
  dat[10] = 0x5B;
  radio_select();
  radio_lld_txrx(11, dat, rdat);
  radio_unselect();

  dat[0] = 0x22; /* Write TX address.*/
  dat[1] = 0x03;
  dat[2] = 0x4D;
  dat[3] = 0x4D;
  dat[4] = 0x03;
  radio_select();
  radio_lld_txrx(5, dat, rdat);
  radio_unselect();

  /* Set to receive mode (TX_EN low).*/
  palWritePad(GPIOB, PB14, PAL_LOW);

  /* Set TRXCE to high to enable transciever.*/
  palWritePad(GPIOB, PB13, PAL_HIGH);
}

void setupChannels(void) {
  uint8_t dat[11];
  uint8_t rdat[11];

  /* Set frequency to 905 MHz = ((905/2) - 422.4) * 10 = 301 = 0x12D
   * might need to change to be backward-compatible
   * hfreq_pll = 1
   * pa_pwr = 11 +10 dBm
   * RxRed_pw = 0
   * auto_retran = 0
   * rx_afw = 100 (4-byte RX address)
   * tx_afw = 100 (4-byte TX address)
   * RX_PW = 1 (1 byte payload)
   * TX_PW = 1 (1 byte payload)
   * RX_address = 0x034DBABE
   * UP_CLK_FREQ = 11 (500 KHz, not used)
   * UP_CLK_EN = 0 (no clock)
   * XOF = 011, 16 MHz
   * CRC_EN = 1 enable
   * CRC_MODE = 0 / 8-bit check

   * payload values:
   * 0: 0x2D
   * 1: 0x0F XX 0 0 11 1 1
   * 2: 0x44
   * 3: 0x01
   * 4: 0x01
   * 5: 0x03
   * 6: 0x4D
   * 7: 0xBA
   * 8: 0xBE
   * 9: 0x5B 0 1 011 0 11 0101 1011
   */

  dat[0] = 0x00; /* Write command to CONFIG from address 0.*/

  /* Might need changing to be backward-compatible with old radios.*/
  dat[1] = 0x2D;
  dat[2] = 0x0F;
  dat[3] = 0x44;
  dat[4] = 0x01;
  dat[5] = 0x01;
  dat[6] = 0x03;
  dat[7] = 0x4D;
  dat[8] = 0x4D;
  dat[9] = 0x03;
  dat[10] = 0x5B;
  radio_select();
  radio_lld_txrx(11, dat, rdat);
  radio_unselect();

  dat[0] = 0x22; /* Write TX address.*/
  dat[1] = 0x03;
  dat[2] = 0x4D;
  dat[3] = 0x4D;
  dat[4] = 0x03;
  radio_select();
  radio_lld_txrx(5, dat, rdat);
  radio_unselect();

}

void radioSend(uint8_t data) {
  uint8_t dat[2];
  uint8_t rdat[2];

  /* Set to tx mode (TX_EN high).*/
  palWritePad(GPIOB, PB14, PAL_HIGH);

  //  setupChannels();

  /* Write TX payload.*/
  dat[0] = 0x20;
  dat[1] = data;
  radio_select();
  radio_lld_txrx(2, dat, rdat);
  radio_unselect();

  /* Set TRXCE to high.*/
  palWritePad(GPIOB, PB13, PAL_HIGH);
  chThdSleepMicroseconds(15);

  /* Set TRXCE to low.*/
  palWritePad(GPIOB, PB13, PAL_LOW);

  chThdSleepMicroseconds(800);

  /* Set to rx mode (TX_EN low).*/
  palWritePad(GPIOB, PB14, PAL_LOW);

  /* Set TRXCE to high (to allow for receiving).*/
  palWritePad(GPIOB, PB13, PAL_HIGH);
  
}
