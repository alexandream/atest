#ifndef AT_ATCHECK_H
#define AT_ATCHECK_H

#include <stdarg.h>

#include "atest.h"

char* at_allocf(const char* fmt, ...);

void at_freef(const char* msg);

int at_asprintf(char** dest, const char* fmt, ...);

#ifdef HAS_VA_COPY
int at_vasprintf(char** dest, const char* fmt, va_list ap);
#endif

AtCheckResult at_eq_long(const char* expr, long value, long expected);

AtCheckResult at_eq_ulong(const char* expr, unsigned long value,
                          unsigned long expected);

AtCheckResult at_eq_ptr(const char* expr, void* value, void* expected );
#define at_eq_int(X, V, E) \
        at_eq_long(X, (int) V, (int) E)

#define at_eq_short(X, V, E) \
        at_eq_long(X, (short) V, (short) E)

#define at_eq_uint(X, V, E) \
        at_eq_ulong(X, (unsigned int) V, (unsigned int) E)

#define at_eq_ushort(X, V, E) \
        at_eq_ulong(X, (unsigned short) V, (unsigned short) E)
#endif
