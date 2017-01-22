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

typedef void  (*AtTestFunc)(void);


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
	int (*check)(AtReporter*, char, const char*, const char*,
	              int, int, int, const char*);
	int (*footer)(AtReporter*, int, int, int);
};

struct AtTest {
	const char* name;
	AtTestFunc test_func;
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


#define at_assert(CHECK)\
	if (!at_assert_f(CHECK, __FILE__, __LINE__)) return

#define at_error(MSG, CLEANUP)\
	do { at_error_f(MSG, CLEANUP, __FILE__, __LINE__); return; } while(0)


#define at_constructor
#endif
