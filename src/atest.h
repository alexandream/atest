#ifndef AT_ATEST_H
#define AT_ATEST_H

#include <stdlib.h>

enum AtCheckStatus {
	AtCheck_SUCCESS,
	AtCheck_FAILURE,
	AtCheck_ERROR
};

enum AtErrors {
	AT_E_SUCCESS = 0,
	AT_E_BADALLOC,
	AT_E_BADIO,
	AT_E_TEST_ERROR,
	AT_E_TEST_FAILURE
};

typedef struct AtCheckResult AtCheckResult;
typedef struct AtContext AtContext;
typedef struct AtReporter AtReporter;
typedef struct AtStreamReporter AtStreamReporter;
typedef struct AtResult AtResult;
typedef struct AtSuite AtSuite;
typedef struct AtTest AtTest;
typedef struct AtIterator AtIterator;
typedef struct AtArrayIterator AtArrayIterator;

typedef void  (*AtTestFunc)(void*);

struct AtIterator {
	int   (*has_next)(AtIterator*);
	void* (*next)(AtIterator*);
	void  (*reset)(AtIterator*);
};



struct AtArrayIterator {
	AtIterator vtable;
	int elem_size;
	int size;
	int current;
	void* array;
};


struct AtCheckResult {
	int status;
	int check_number;
	const char* message;
	const char* file_name;
	int line_number;
	void (*clean_up)(const char*);
};


struct AtReporter {
	int (*header)(AtReporter*, const char*);
	int (*check)(AtReporter*, char, const char*, const char*, const char*,
	              int, int, int, const char*);
	int (*footer)(AtReporter*, int, int, int);
};

struct AtTest {
	const char* name;
	AtTestFunc test_func;
	AtIterator* iterator;
};

/* Global variables */
extern AtResult* at_current_result;
extern AtContext* at_current_context;

/* Exposed functions */

int at_add_test(AtSuite*, AtTest*);

void* at_alloc(size_t);

int at_assert_f(AtCheckResult check, const char* file_name, int line_number);

void at_error_f(const char* message, void (*clean_up)(const char*),
                const char* file_name, int line_number);

AtStreamReporter* at_new_stream_reporter(const char* file_name);

AtSuite* at_new_suite(const char* name, AtTestFunc construct,
                      AtTestFunc destruct, AtTestFunc setup,
                      AtTestFunc teardown);

void at_destroy_suite(AtSuite* suite);

void at_run_suite(AtSuite* suite, AtReporter* reporter);

AtCheckResult at_make_error(const char* msg, void (*clean_up) (const char*));

AtCheckResult at_make_failure(const char* msg, void (*clean_up) (const char*));

AtCheckResult at_make_success(void);

int at_array_iterator_has_next(AtIterator*);

void* at_array_iterator_next(AtIterator*);

void at_array_iterator_reset(AtIterator*);


#define at_assert(CHECK)\
	if (!at_assert_f(CHECK, __FILE__, __LINE__)) return

#define at_error(MSG, CLEANUP)\
	do { at_error_f(MSG, CLEANUP, __FILE__, __LINE__); return; } while(0)



/* at_sf: Stringify -- Indirect stringification to use expansion. */
#define at_sf2(x) #x
#define at_sf(x) at_sf2(x)
/* at_pt: Paste Tokens */
#define at_pt2(x, y) x ## y
#define at_pt(x, y) at_pt2(x, y)

#define _at_test_function(NAME) static void NAME(void*)
#define at_test(NAME)                    \
_at_test_function(at_pt(_at_tf_,NAME));  \
static AtTest NAME = {                   \
	at_sf(NAME),                         \
	at_pt(_at_tf_,NAME),                 \
	NULL                                 \
};                                       \
_at_test_function(at_pt(_at_tf_,NAME))


#define at_constructor(NAME) _at_test_function(NAME)
#define at_destructor(NAME)  _at_test_function(NAME)
#define at_setup(NAME)       _at_test_function(NAME)
#define at_teardown(NAME)    _at_test_function(NAME)
#define at_teardown(NAME)    _at_test_function(NAME)

#endif
