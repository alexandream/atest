#include <stdlib.h>
#include "atcheck.h"

#define EQ_CHECK_BODY(EXPRESSION, VALUE, EXPECTED, COMPARISON, FORMAT)   \
	do {                                                                 \
		if (! (COMPARISON) ) {                                           \
			const char* _at_msg =                                        \
				at_allocf("Expression (%s) expected to have value "      \
						  FORMAT " but found to have value " FORMAT ".", \
						  EXPRESSION, EXPECTED, VALUE);                  \
			if (_at_msg == NULL) {                                       \
				const char* _at_error_msg =                              \
					"Allocation error during failed check";              \
				return                                                   \
					at_make_error(_at_error_msg, NULL);                  \
			}                                                            \
			return at_make_failure(_at_msg, at_freef);                   \
		}                                                                \
		return at_make_success();                                        \
	} while (0)


AtCheckResult at_eq_long(const char* expr, long value, long expected) {
	EQ_CHECK_BODY(expr, value, expected, (value == expected), "%ld");
}


AtCheckResult at_eq_ulong(const char* expr, unsigned long value,
                          unsigned long expected) {
	EQ_CHECK_BODY(expr, value, expected, (value == expected), "%lu");
}

AtCheckResult at_eq_ptr(const char* expr, void* value, void* expected ) {
	EQ_CHECK_BODY(expr, value, expected, (value == expected), "%p");
}
