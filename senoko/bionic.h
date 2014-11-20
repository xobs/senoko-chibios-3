#ifndef __BIONIC_H__
#define __BIONIC_H__

#include "ch.h"

#define ULONG_MAX	4294967295UL

int strcasecmp(const char *s1, const char *s2);
void *memcpy(void *dst0, const void *src0, size_t length);
void *memset(void *dst0, int val, size_t length);
unsigned long strtoul(const char *nptr, char **endptr, int base);
long strtol(const char *cp, char **endptr, unsigned int base);
size_t strlen (const char *__s);
size_t strnlen (const char *__string, size_t __maxlen);

#endif /* __BIONIC_H__ */
