#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "atest.h"

AtResult* at_current_result;
AtContext* at_current_context;

#define ARRAY_INITIAL_SIZE 64

#define ARRAY_BODY(TYPE)\
	int size;           \
	int count;          \
	TYPE* pool

typedef struct TestArray TestArray;
typedef struct FailureArray FailureArray;
typedef struct TestSummary TestSummary;

struct TestArray {
	ARRAY_BODY(AtTest);
};


struct TestSummary {
	int tests;
	int errors;
	int failures;
};


struct FailureArray {
	ARRAY_BODY(AtCheckResult);
};


struct AtResult {
	const char* iteration_name;
	int has_error;
	AtCheckResult error;
	int checks_count;
	FailureArray failures;
};


struct AtSuite {
	const char* name;
	AtTestFunc construct;
	AtTestFunc setup;
	AtTestFunc teardown;
	AtTestFunc destruct;
	TestArray tests;
};


struct AtStreamReporter {
	AtReporter vtable;
	FILE* stream;
};

static AtReporter  STREAM_REPORTER_VTABLE;
static AtReporter* STREAM_REPORTER_VTABLE_PTR = NULL;

static AtStreamReporter* STDOUT_REPORTER_PTR = NULL;
static AtStreamReporter  STDOUT_REPORTER;

/* Internal function declarations. */
static
AtReporter* stdout_reporter();

static
AtReporter stream_reporter_vtable();

static
int process_result(AtCheckResult* result, AtResult *test_result,
                    const char* file_name, int line_number);

static
int testarray_add(TestArray* array, AtTest* test);

static
int testarray_init(TestArray*);

static
void testarray_destruct(TestArray* array);

static
int failarray_add(FailureArray* array, AtCheckResult* failure);

static
int failarray_init(FailureArray* array);

static
void failarray_destruct(FailureArray* array);

static
int report_header(AtReporter* reporter, AtSuite* suite);

static
int report_constructor(AtReporter* reporter, AtResult* result);

static
int report_error(AtReporter* reporter, AtTest* test, AtResult* result);

static
int report_failure(AtReporter* reporter, AtTest* test, AtResult* result);

static
int report_footer(AtReporter* reporter, int tests, int errors, int failures);

static
int run_constructor(AtSuite* suite, AtReporter* reporter);

static
TestSummary run_test(AtSuite* suite, AtTest* test, AtReporter* reporter);

static
int init_result(AtResult* result);

static
void destruct_result(AtResult* result);

static
AtCheckResult make_check_result(int status, const char* message,
                                void (*clean_up)(const char*));

/* Exposed functions */


int at_add_test(AtSuite* suite, AtTest* test) {
	return testarray_add(&suite->tests, test);
}


void* at_alloc(size_t size) {
	return malloc(size);
}


int at_assert_f(AtCheckResult check, const char* file_name, int line_number) {
	AtResult* result = at_current_result;
	process_result(&check, result, file_name, line_number);

	return check.status == AtCheck_SUCCESS;
}


void at_error_f(const char* message, void (*clean_up)(const char*),
                const char* file_name, int line_number) {
	AtResult* result = at_current_result;
	AtCheckResult check = at_make_error(message, clean_up);

	process_result(&check, result, file_name, line_number);
}


AtStreamReporter* at_new_stream_reporter(const char* file_name) {
	AtStreamReporter* result;
	FILE* stream = fopen(file_name, "w");
	if (stream == NULL) {
		return NULL;
	}

	result = malloc(sizeof(AtStreamReporter));
	if (result == NULL) {
		fclose(stream);
	}

	result->vtable = stream_reporter_vtable();
	result->stream = stream;

	return result;
}


AtSuite* at_new_suite(const char* name, AtTestFunc construct,
                      AtTestFunc destruct, AtTestFunc setup,
                      AtTestFunc teardown) {
	AtSuite* result = malloc(sizeof(AtSuite));
	if (result == NULL) {
		return NULL;
	}

	result->name = name;
	result->construct = construct;
	result->destruct = destruct;
	result->setup = setup;
	result->teardown = teardown;

	if (testarray_init(&result->tests) != AT_E_SUCCESS) {
		free(result);
		return NULL;
	}
	return result;
}


void at_destroy_suite(AtSuite* suite) {
	testarray_destruct(&suite->tests);
	free(suite);
}


void at_run_suite(AtSuite* suite, AtReporter* reporter) {
	int i;
	int tests = 0;
	int failures = 0;
	int errors = 0;

	if (reporter == NULL) {
		reporter = stdout_reporter();
	}

	report_header(reporter, suite);

	if (suite->construct != NULL) {
		if (run_constructor(suite, reporter) != AT_E_SUCCESS) {
			/* Constructor has error. Abort and go home in shame. */
			return;
		}
	}

	for (i = 0; i < suite->tests.count; i++) {
		AtTest* test = &suite->tests.pool[i];
		TestSummary summary = { 0 };

		summary = run_test(suite, test, reporter);
		tests += summary.tests;
		errors += summary.errors;
		failures += summary.failures;
	}
	report_footer(reporter, tests, errors, failures);

	if (suite->destruct != NULL) {
		suite->destruct(NULL);
	}
}

AtCheckResult at_make_error(const char* msg, void (*clean_up) (const char*)) {
	return make_check_result(AtCheck_ERROR, msg, clean_up);
}


AtCheckResult at_make_failure(const char* msg,
                              void (*clean_up) (const char*)) {
	return make_check_result(AtCheck_FAILURE, msg, clean_up);
}


AtCheckResult at_make_success(void) {
	return make_check_result(AtCheck_SUCCESS, NULL, NULL);
}



int at_array_iterator_has_next(AtIterator* iter) {
	AtArrayIterator* self = (AtArrayIterator*) iter;
	return self->current < self->size;
}


void* at_array_iterator_next(AtIterator* iter) {
	AtArrayIterator* self = (AtArrayIterator*) iter;
	return ((char*) self->array) + self->elem_size * self->current++;
}


void at_array_iterator_reset(AtIterator* iter) {
	AtArrayIterator* self = (AtArrayIterator*) iter;
	self->current = 0;
}


/* Internal function definitions. */

#define ARRAY_ADD_FUNC_BODY(ARR_TYPE, ARR_VAR, TYPE, ELEM)         \
	do {                                                           \
		ARR_TYPE* _array = ARR_VAR;                                \
		TYPE _elem = (ELEM);                                       \
		if (_array->count >= _array->size) {                       \
			int new_size = _array->size * 2;                       \
			size_t new_mem_size = new_size * sizeof(TYPE);         \
			void** new_pool = realloc(_array->pool, new_mem_size); \
			if (new_pool == NULL) {                                \
				return AT_E_BADALLOC;                              \
			}                                                      \
			_array->size = new_size;                               \
		}                                                          \
		_array->pool[_array->count] = _elem;                       \
		_array->count++;                                           \
		return AT_E_SUCCESS;                                       \
	} while(0)


#define ARRAY_INIT_FUNC_BODY(ARR_TYPE, ARR_VAR, TYPE)  \
	do {                                               \
		ARR_TYPE* _array = ARR_VAR;                    \
		TYPE *pool =                                   \
			malloc(sizeof(TYPE) * ARRAY_INITIAL_SIZE); \
		if (pool == NULL) {                            \
			return AT_E_BADALLOC;                      \
		}                                              \
		_array->count = 0;                             \
		_array->size = ARRAY_INITIAL_SIZE;             \
		_array->pool = pool;                           \
		return AT_E_SUCCESS;                           \
	} while (0)


static
size_t compute_encoded_length(const char* input) {
	size_t original_length = strlen(input);
	size_t i;
	size_t result = 0;

	for (i = 0; i < original_length; i++) {
		char c = input[i];
		result += (c == '\\' || c == '\r' || c == '\n') ? 2 : 1;
	}

	return result;
}


static
char* encode_crlf(const char* input) {
	size_t original_length = strlen(input);
	size_t encoded_length = compute_encoded_length(input);
	size_t i;
	size_t j;
	char* result;

	result = malloc(sizeof(char) * (encoded_length+1));
	if (result == NULL) {
		return NULL;
	}

	for (i = 0, j = 0; i < original_length; i++, j++) {
		char c = input[i];

		if (c == '\\' || c == '\r' || c == '\n') {
			result[j++] = '\\';
			result[j] = (c == '\\') ? '\\'
			          : (c == '\r') ? 'r'
			          : 'n'; /* can only be \n */
		}
		else {
			result[j] = c;
		}
	}
	result[j] = '\0';
	return result;
}


static
int fprintf_header(AtReporter* reporter, const char* name) {
	AtStreamReporter* self = (AtStreamReporter*) reporter;
	int status = fprintf(self->stream, "%s\n", name);
	if (status < 0) {
		return AT_E_BADIO;
	}
	return AT_E_SUCCESS;
}


static
int fprintf_check(AtReporter* reporter, char type, const char* test,
	              const char* iteration, const char* file, int line,
	              int check, int checks, const char* message) {
	AtStreamReporter* self = (AtStreamReporter*) reporter;
	char* oneline_message = encode_crlf(message);
	int status;
	const char* lpar = (iteration)?" (":"";
	const char* rpar = (iteration)?")":"";
	iteration = (iteration)? iteration : "";

	if (oneline_message == NULL) {
		return AT_E_BADALLOC;
	}

	status = fprintf(self->stream, "%c\t%s%s%s%s\t%s\t%d\t%d\t%d\t%s\n",
	                     type, test, lpar, iteration, rpar, file, line,
	                     check, checks, oneline_message);

	free(oneline_message);

	if (status < 0) {
		return AT_E_BADIO;
	}
	return AT_E_SUCCESS;
}


static
int fprintf_footer(AtReporter* reporter, int tests, int errors, int fails) {
	AtStreamReporter* self = (AtStreamReporter*) reporter;
	int status = fprintf(self->stream, "#\t%d\t%d\t%d\n", tests, errors, fails);
	if (status < 0) {
		return AT_E_BADIO;
	}
	return AT_E_SUCCESS;
}


static
AtReporter* stdout_reporter() {
	if (STDOUT_REPORTER_PTR == NULL) {
		STDOUT_REPORTER.vtable = stream_reporter_vtable();
		STDOUT_REPORTER.stream = stdout;
		STDOUT_REPORTER_PTR = &STDOUT_REPORTER;
	}
	return (AtReporter*) STDOUT_REPORTER_PTR;
}


static
AtReporter stream_reporter_vtable() {
	if (STREAM_REPORTER_VTABLE_PTR == NULL) {
		STREAM_REPORTER_VTABLE.header = fprintf_header;
		STREAM_REPORTER_VTABLE.check  = fprintf_check;
		STREAM_REPORTER_VTABLE.footer = fprintf_footer;
		STREAM_REPORTER_VTABLE_PTR = &STREAM_REPORTER_VTABLE;
	}
	return STREAM_REPORTER_VTABLE;
}


static
int process_result(AtCheckResult* result, AtResult *test_result,
                   const char* file_name, int line_number) {
	result->check_number = test_result->checks_count++;
	result->file_name = file_name;
	result->line_number = line_number;

	if (result->status == AtCheck_FAILURE) {
		int insert_err = failarray_add(&test_result->failures, result);
		if (insert_err != 0) {
			result->message = "Allocation error while inserting failure.";
			result->status = AtCheck_ERROR;
		}
	}
	if (result->status == AtCheck_ERROR) {
		test_result->has_error = 1;
		test_result->error = *result;
	}
	return AT_E_SUCCESS;
}


static
int testarray_add(TestArray* array, AtTest* test) {
	ARRAY_ADD_FUNC_BODY(TestArray, array, AtTest, *test);
}


static
int testarray_init(TestArray* array) {
	ARRAY_INIT_FUNC_BODY(TestArray, array, AtTest);
}


static
void testarray_destruct(TestArray* array) {
	free(array->pool);
}


static
int failarray_add(FailureArray* array, AtCheckResult* failure) {
	ARRAY_ADD_FUNC_BODY(FailureArray, array, AtCheckResult, *failure);
}


static
int failarray_init(FailureArray* array) {
	ARRAY_INIT_FUNC_BODY(FailureArray, array, AtCheckResult);
}


static
void failarray_destruct(FailureArray* array) {
	free(array->pool);
}


static
int report_header(AtReporter* reporter, AtSuite* suite) {
	return reporter->header(reporter, suite->name);

}


static
int report_check(AtReporter* reporter, char type, const char* test_name,
                 const char* iteration_name, int checks_count,
                 AtCheckResult* check) {
	return reporter->check(reporter, type, test_name, iteration_name,
	                       check->file_name, check->line_number,
	                       check->check_number +1, checks_count,
	                       check->message);
}


static
int report_constructor(AtReporter* reporter, AtResult* result) {
	AtCheckResult* check = & result->error;
	return report_check(reporter, 'E', "<constructor>", NULL,
	                    result->checks_count, check);
}


static
int report_error(AtReporter* reporter, AtTest* test, AtResult* result) {
	AtCheckResult* check = & result->error;
	return report_check(reporter, 'E', test->name, result->iteration_name,
	                    result->checks_count, check);
}


static
int report_failure(AtReporter* reporter, AtTest* test, AtResult* result) {
	const char* test_name = test->name;
	const char* iteration_name = result->iteration_name;
	int failures_count = result->failures.count;
	int checks_count = result->checks_count;
	int i;

	for (i = 0; i < failures_count; i++) {
		AtCheckResult* check_result = & result->failures.pool[i];
		int status =
			report_check(reporter, 'F', test_name, iteration_name,
			             checks_count, check_result);
		if (status != AT_E_SUCCESS) {
			return status;
		}
	}
	return AT_E_SUCCESS;
}


static
int report_footer(AtReporter* reporter, int tests, int errors, int failures) {
	return reporter->footer(reporter, tests, errors, failures);
}


static
int run_with_new_result(int (*func)(void*), void* data) {
	int status = AT_E_SUCCESS;
	AtResult* last_result;
	AtResult result;
	int init_result_status;


	init_result_status = init_result(&result);
	if (init_result_status != AT_E_SUCCESS) {
		return init_result_status;
	}

	last_result = at_current_result;
	at_current_result = &result;

	status = func(data);

	destruct_result(&result);
	at_current_result = last_result;

	return status;
}


static
int do_run_constructor(void* data) {
	AtSuite* suite       = (AtSuite*)    ((void**) data)[0];
	AtReporter* reporter = (AtReporter*) ((void**) data)[1];

	suite->construct(NULL);
	if (at_current_result->has_error) {
		/* Constructor has error. Abort everything and go home in shame. */
		report_constructor(reporter, at_current_result);
		return AT_E_TEST_ERROR;
	}
	return AT_E_SUCCESS;
}


static
int run_constructor(AtSuite* suite, AtReporter* reporter) {
	void* data[2];
	data[0] = suite;
	data[1] = reporter;
	return run_with_new_result(do_run_constructor, data);
}


static
int do_run_test(void* data) {
	AtSuite* suite = (AtSuite*) ((void**) data)[0];
	AtTest* test = (AtTest*) ((void**) data)[1];
	AtReporter* reporter = (AtReporter*) ((void**) data)[2];
	int* i = (int*) ((void**) data)[3];
	void* test_data = ((void**) data)[4];
	AtResult* result = at_current_result;
	char name_buffer[32];
	
	if (*i >= 0) {
		sprintf(name_buffer, "%d", *i);
		at_current_result->iteration_name = name_buffer;
	}

	if (suite->setup != NULL) {
		suite->setup(test_data);
	}

	if (!result->has_error) {
		test->test_func(test_data);
	}

	if (suite->teardown != NULL) {
		suite->teardown(test_data);
	}

	if (result->has_error) {
		report_error(reporter, test, result);
		return AT_E_TEST_ERROR;
	}
	else if (result->failures.count > 0) {
		report_failure(reporter, test, result);
		return AT_E_TEST_FAILURE;
	}
	return AT_E_SUCCESS;
}


static
TestSummary run_test(AtSuite* suite, AtTest* test, AtReporter* reporter) {
	void* data[5];
	int status;
	int i = -1;
	TestSummary result = {0};
	data[0] = suite;
	data[1] = test;
	data[2] = reporter;
	data[3] = &i;

	if (test->iterator) {
		AtIterator* iterator = test->iterator;
		iterator->reset(iterator);
		while (iterator->has_next(iterator)) {
			i++;
			data[4] = iterator->next(iterator);
			status = run_with_new_result(do_run_test, data);
			result.tests++;
			if (status == AT_E_TEST_ERROR) {
				result.errors++;
			}
			else if (status == AT_E_TEST_FAILURE) {
				result.failures++;
			}
		}
	}
	else {
		data[4] = NULL;
		status = run_with_new_result(do_run_test, data);
		result.tests++;
		if (status == AT_E_TEST_ERROR) {
			result.errors++;
		}
		else if (status == AT_E_TEST_FAILURE) {
			result.failures++;
		}
	}
	return result;
}

static
int init_result(AtResult* result) {
	result->iteration_name = NULL;
	result->has_error = 0;
	result->error.status = AtCheck_SUCCESS;
	result->checks_count = 0;
	return failarray_init(&result->failures);
}


static
void destruct_result(AtResult* result) {
	if (result->has_error) {
		if (result->error.clean_up != NULL) {
			result->error.clean_up(result->error.message);
		}
	}
	if (result->failures.count > 0) {
		int i;
		for (i = 0; i < result->failures.count; i++) {
			AtCheckResult* check = &result->failures.pool[i];
			if (check->clean_up != NULL) {
				check->clean_up(check->message);
			}
		}
	}
	failarray_destruct(&result->failures);
}


static
AtCheckResult make_check_result(int status, const char* message,
                                void (*clean_up)(const char*)) {
	AtCheckResult result;

	result.status = status;
	result.message = message;
	result.clean_up = clean_up;

	return result;
}
