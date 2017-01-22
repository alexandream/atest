#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "atcheck.h"

#ifdef HAS_VASPRINTF
int vasprintf(char **strp, const char *fmt, va_list ap);
#endif 

#if !defined(HAS_VASPRINTF) && !defined(HAS_VSNPRINTF)
#define NEED_COMPUTE_FORMAT_LENGTH
#endif 


#ifdef NEED_COMPUTE_FORMAT_LENGTH
static
int compute_format_length(const char* format, va_list ap);
#endif

static 
int do_vasprintf(char** dest, const char* fmt, va_list ap1, va_list ap2);

int at_asprintf(char** dest, const char* fmt, ...) {
	int length;
	va_list ap1;
	va_list ap2;
	
	va_start(ap1, fmt);
	va_start(ap2, fmt);
	length = do_vasprintf(dest, fmt, ap1, ap2);
	va_end(ap2);
	va_end(ap1);

	return length;
}

char* at_allocf(const char* fmt, ...) {
	char* result = NULL;
	int length;
	va_list ap1;
	va_list ap2;

	va_start(ap1, fmt);
	va_start(ap2, fmt);
	length = do_vasprintf(&result, fmt, ap1, ap2);
	va_end(ap2);
	va_end(ap1);

	return (length >= 0) ? result : NULL;
}


#ifdef HAS_VA_COPY
int at_vasprintf(char** dest, const char* fmt, va_list ap) {
	int length;
	va_list ap_copy;
	
	va_copy(ap_copy, ap);
	length = do_vasprintf(dest, fmt, ap, ap_copy);
	va_end(ap_copy);
	return length;
}
#endif


void at_freef(const char* msg) {
	free((void*) msg);
}	


#ifdef NEED_COMPUTE_FORMAT_LENGTH

#define CONVERSION_BUFFER_SIZE 2048
#define MAX_SPEC_SIZE 16
#define VALID_TYPES "AEFGXacdefginopsux%"
static
char BUFFER[CONVERSION_BUFFER_SIZE];


static
int get_part_length(const char* part, va_list ap, const char** next_part) {
	char spec[MAX_SPEC_SIZE];
	int result = 0;
	if (*part != '%') {
		while (*part && *part != '%') {
			result++;
			part++;
		}
		*next_part = part;
	}
	else {
		size_t fmt_len = strcspn(part+1, VALID_TYPES) + 1;
		char type = part[fmt_len];

		*next_part = part + fmt_len + 1;
		
		switch(type) {
			case 's':
				result = strlen(va_arg(ap, const char*));
				break;
			case '%':
				result = 1;
				break;
			default:
				if (fmt_len + 1 >= MAX_SPEC_SIZE) {
					/* We can't put it into our buffer. We're done. */
					return -1;
				}
				else if (!strchr(VALID_TYPES, type)) {
					/* Unsupported type. Give up. */
					return -2;
				}
				strncpy(spec, part, fmt_len+1);
				spec[fmt_len+1] = '\0';
				{
					int len = vsprintf(BUFFER, spec, ap);
					if (len < 0) {
						/* Got an error trying to figure the len. Give up. */
						return -3;
					}

					result = len;
				}
				break;
		}
				
	}
	return result;
}
#endif


#ifdef NEED_COMPUTE_FORMAT_LENGTH
static
int compute_format_length(const char* format, va_list ap) {
#ifdef HAS_VSNPRINTF
	return vsnprintf(NULL, 0, format, ap);
#else
	int result = 0;
	const char* part = format;
	while(*part) {
		const char* next_part = NULL;
		int part_len;
		part_len = get_part_length(part, ap, &next_part);
		if (part_len < 0) {
			return -1;
		}
		
		result += part_len;
		part = next_part;
	}
	return result;
#endif
}
#endif

static 
int do_vasprintf(char** dest, const char* fmt, va_list ap1, va_list ap2) {
	int length;
#ifdef HAS_VASPRINTF
	length = vasprintf(dest, fmt, ap1);
#else
	char* buf;
	int expected_length;
	expected_length = compute_format_length(fmt, ap1);

	if (expected_length < 0) {
		return expected_length;
	}

	buf = malloc(sizeof(char) * (expected_length + 1));
	if (buf == NULL) {
		/* Couldn't allocate, error out. */
		return -11;
	}

	length = vsprintf(buf, fmt, ap2);

	if (length < 0) {
		free(buf);
		return -12;
	}

	*dest = buf;
#endif

	return length;
}
