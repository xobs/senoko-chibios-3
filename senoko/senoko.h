#ifndef __SENOKO_H__
#define __SENOKO_H__

extern const char *gitversion;

#define SENOKO_OS_VERSION_MAJOR 2
#define SENOKO_OS_VERSION_MINOR 0

#define SENOKO_I2C_SLAVE_ADDR (0x20)

#define serialDriver (&SD1)
#define stream ((BaseSequentialStream *)&SD1)

#endif /* __SENOKO_H__ */
