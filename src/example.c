#include <string.h>
char* strdup(const char*);

#include "atest.h"
#include "atcheck.h"

const char* SUITE_NAME = "Strdup";


char* DUPE = NULL;

void setup(void) {
	DUPE = NULL;
}


void teardown(void) {
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

void _t1(void) {
	at_assert(success());
}
AtTest succ = { "Success", _t1 };


void _t2(void) {
	at_assert(failure());
}
AtTest fail = { "Failure", _t2 };


void _t3(void) {
	at_assert(error());
}
AtTest err = { "Error", _t3 };


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
