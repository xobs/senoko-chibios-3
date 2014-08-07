#ifndef __SENOKO_H__
#define __SENOKO_H__

extern const char *gitversion;

#define SENOKO_OS_VERSION_MAJOR 2
#define SENOKO_OS_VERSION_MINOR 0

<<<<<<< HEAD
#define SENOKO_I2C_SLAVE_ADDR (0x20)

#define serialDriver (&SD1)
#define stream_driver ((BaseSequentialStream *)serialDriver)
extern void *stream;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))
#endif
=======
#define serialDriver (&SD1)
#define stream ((BaseSequentialStream *)&SD1)

>>>>>>> 4177a65a07b748bb28ca7f5533e1ca3dadba5e2c

#endif /* __SENOKO_H__ */
