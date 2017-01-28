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

AtCheckResult at_eq_str(const char* expr, const char* value,
                        const char* expected);

#define at_eq_int(X, V, E) \
        at_eq_long(X, (int) V, (int) E)

#define at_eq_short(X, V, E) \
        at_eq_long(X, (short) V, (short) E)

#define at_eq_uint(X, V, E) \
        at_eq_ulong(X, (unsigned int) V, (unsigned int) E)

#define at_eq_ushort(X, V, E) \
        at_eq_ulong(X, (unsigned short) V, (unsigned short) E)

/* Helper macros that automatically print the expression. */
#define AT_EQ_INT(V, E)    at_eq_int(#V, V, E)
#define AT_EQ_UINT(V, E)   at_eq_uint(#V, V, E)
#define AT_EQ_SHORT(V, E)  at_eq_short(#V, V, E)
#define AT_EQ_USHORT(V, E) at_eq_ushort(#V, V, E)
#define AT_EQ_LONG(V, E)   at_eq_long(#V, V, E)
#define AT_EQ_ULONG(V, E)  at_eq_ulong(#V, V, E)

#define AT_EQ_PTR(V, E) at_eq_ptr(#V, V, E)
#define AT_EQ_STR(V, E) at_eq_str(#V, V, E)

#define AT_IS_NULL(V) AT_EQ_PTR(V, NULL)
#endif
