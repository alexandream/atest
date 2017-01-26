#include <string.h>
char* strdup(const char*);

#include "atest.h"
#include "atcheck.h"

const char* SUITE_NAME = "Strdup";


char* DUPE = NULL;

void setup(void* _) {
	DUPE = NULL;
}


void teardown(void* _) {
	free(DUPE);
}

AtCheckResult success() {
	return at_make_success();
}

AtCheckResult failure() {
	int x, y;
	return at_eq_ptr("Wakka", &x, &y);
}

AtCheckResult error() {
	return at_make_error("We see\nthe error of\rour \\ways.", NULL);
}

void _t1(void* _) {
	at_assert(success());
}
AtTest succ = { "Success", _t1 };


void _t2(void* _) {
	at_assert(failure());
}
AtTest fail = { "Failure", _t2 };


int test_data[] = { 1, 2, 3 };
AtArrayIterator iterwhatever = {
	{
		at_array_iterator_has_next,
		at_array_iterator_next,
		at_array_iterator_reset
	},
	sizeof(test_data[0]),
	sizeof(test_data)/sizeof(test_data[0]),
	0,
	(void*) test_data

};

void _t3(void* _) {
	at_assert(error());
}
AtTest err = { "Error", _t3, (AtIterator*) &iterwhatever};


int main(int argc, char** argv) {
	AtStreamReporter* reporter = NULL;
	AtSuite* suite = 
		at_new_suite("Strdup", NULL, NULL, setup, teardown);

	
	at_add_test(suite, &succ);
	at_add_test(suite, &fail);
	at_add_test(suite, &err);
	
	if (argc > 1) {
		const char* file_name = argv[1];
		reporter = at_new_stream_reporter(file_name);
	}
		
	at_run_suite(suite, (AtReporter*) reporter);
	return 0;
}
