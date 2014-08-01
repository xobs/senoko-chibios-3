#ifndef __PHAGE_H__
#define __PHAGE_H__

extern const char *gitversion;
extern void *stream;

#define PHAGE_OS_VERSION_MAJOR 2014
#define PHAGE_OS_VERSION_MINOR 0

#define serialDriver (&SD1)
#define stream_driver ((BaseSequentialStream *)&SD1)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))
#endif

#endif /* __PHAGE_H__ */
