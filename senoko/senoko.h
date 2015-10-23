#ifndef __SENOKO_H__
#define __SENOKO_H__

extern const char *gitversion;

#define SENOKO_OS_VERSION_MAJOR 2
#define SENOKO_OS_VERSION_MINOR 31

#define SENOKO_I2C_SLAVE_ADDR (0x20)

#define serialDriver (&SD1)
#define stream_driver ((BaseSequentialStream *)serialDriver)
extern void *stream;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))
#endif

extern uint32_t senoko_uptime;

#endif /* __SENOKO_H__ */
