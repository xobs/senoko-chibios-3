#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <ctype.h>

#define EEPROM_ADDRESS (0x20)
#define I2C_BUS "/dev/i2c-0"

#include "senoko-slave.h"

struct senoko_dev {
	int fd;
	int addr;
};

int print_hex_offset(uint8_t *block, int count, int offset) {
    int byte;
    count += offset;
    block -= offset;
    for ( ; offset<count; offset+=16) {
        printf("%08x ", offset);

        for (byte=0; byte<16; byte++) {
            if (byte == 8)
                printf(" ");
            if (offset+byte < count)
                printf(" %02x", block[offset+byte]&0xff);
            else
                printf("   ");
        }

        printf("  |");
        for (byte=0; byte<16 && byte+offset<count; byte++)
            printf("%c", isprint(block[offset+byte]) ?
                                    block[offset+byte] :
                                    '.');
        printf("|\n");
    }
    return 0;
}

int print_hex(uint8_t *block, int count) {
    return print_hex_offset(block, count, 0);
}

int senoko_read_i2c(struct senoko_dev *dev, int addr, void *data, int count) {
	struct i2c_rdwr_ioctl_data session;
	struct i2c_msg messages[2];
	char set_addr_buf[1];

	memset(set_addr_buf, 0, sizeof(set_addr_buf));
	memset(data, 0, count);

	set_addr_buf[0] = addr;

	messages[0].addr = dev->addr;
	messages[0].flags = 0;
	messages[0].len = sizeof(set_addr_buf);
	messages[0].buf = set_addr_buf;

	messages[1].addr = dev->addr;
	messages[1].flags = I2C_M_RD;
	messages[1].len = count;
	messages[1].buf = data;

	session.msgs = messages;
	session.nmsgs = 2;

	if(ioctl(dev->fd, I2C_RDWR, &session) < 0) {
		perror("Unable to communicate with i2c device");
		exit(1);
		return 1;
	}

	/*
	messages[0].addr = dev->addr;
	messages[0].flags = I2C_M_RD;
	messages[0].len = count;
	messages[0].buf = data;

	session.msgs = messages;
	session.nmsgs = 1;

	if(ioctl(dev->fd, I2C_RDWR, &session) < 0) {
		perror("Unable to communicate with i2c device");
		exit(1);
		return 1;
	}
	*/

	return 0;
}

int senoko_write_i2c(struct senoko_dev *dev, int addr,
                     const void *data, int count) {
	struct i2c_rdwr_ioctl_data session;
	struct i2c_msg messages[1];
	char data_buf[1 + count];

	data_buf[0] = addr;
	memcpy(&data_buf[1], data, count);

	messages[0].addr = dev->addr;
	messages[0].flags = 0;
	messages[0].len = sizeof(data_buf);
	messages[0].buf = data_buf;

	session.msgs = messages;
	session.nmsgs = 1;

	if(ioctl(dev->fd, I2C_RDWR, &session) < 0) {
		perror("Unable to communicate with i2c device");
		exit(1);
		return 1;
	}

	return 0;
}

struct senoko_dev *senoko_open(char *path, int addr) {
	struct senoko_dev *dev;

	dev = malloc(sizeof(*dev));
	if (!dev) {
		perror("Unable to alloc data");
		goto malloc_err;
	}

	memset(dev, 0, sizeof(*dev));

	dev->fd = open(path, O_RDWR);
	if (dev->fd == -1) {
		perror("Unable to open i2c device");
		goto open_err;
	}

	dev->addr = addr;

	return dev;

open_err:
	free(dev);
malloc_err:
	return NULL;
}

int senoko_close(struct senoko_dev **dev) {
	if (!dev || !*dev)
		return 0;
	close((*dev)->fd);
	free(*dev);
	*dev = NULL;
	return 0;
}

int main(int argc, char **argv) {
	struct senoko_dev *dev;
	int ch;
	int writing = 0;
	char *tmp;
	uint8_t bfr[23];
	struct i2c_registers reg;

	uint32_t uptime;
	uint8_t signature;
	uint8_t ver_major;
	uint8_t ver_minor;
	uint8_t board_type;
	uint8_t wdt_secs;
	uint8_t power;

	dev = senoko_open(I2C_BUS, EEPROM_ADDRESS);
	if (!dev)
		return 1;

	senoko_read_i2c(dev, 4, &uptime, sizeof(uptime));
	senoko_read_i2c(dev, 1, &ver_major, sizeof(ver_major));
	senoko_read_i2c(dev, 2, &ver_minor, sizeof(ver_minor));
	senoko_read_i2c(dev, 0, &signature, sizeof(signature));
	senoko_read_i2c(dev, 3, &board_type, sizeof(board_type));
	senoko_read_i2c(dev, 0x28, &wdt_secs, sizeof(wdt_secs));
	senoko_read_i2c(dev, 0x0f, &power, sizeof(power));

	printf("Signature: %c (0x%02x)\n", signature, signature);
	printf("Version %d.%d (%s)\n", ver_major, ver_minor, (board_type & 1) ? "full" : "abbreviated");
	printf("Uptime: %d.%d seconds\n", uptime / 1000, uptime - ((uptime / 1000) * 1000));
	printf("Watchdog is %s\n", (power & REG_POWER_WDT_STATE) ? "enabled" : "disabled");
	printf("Watchdog fires in %d seconds\n", wdt_secs);

	wdt_secs = 10;
	senoko_write_i2c(dev, 0x28, &wdt_secs, sizeof(wdt_secs));

	printf("Power register: 0x%02x\n", power);
	if ((power & REG_POWER_STATE_MASK) == REG_POWER_STATE_ON)
		printf("Powered on\n");
	else if ((power & REG_POWER_STATE_MASK) == REG_POWER_STATE_OFF)
		printf("Powered off\n");
	else
		printf("Unknown power state\n");

#if 0
	power &= ~REG_POWER_KEY_MASK;
	power |= REG_POWER_KEY_WRITE;
	power |= REG_POWER_WDT_ENABLE;
//	power &= ~REG_POWER_STATE_MASK;
//	power |= REG_POWER_STATE_REBOOT;
	printf("Ensuring WDT is on (0x%02x)...\n", power);
	senoko_write_i2c(dev, 0x0f, &power, sizeof(power));
#endif

	senoko_close(&dev);

	return 0;
}
