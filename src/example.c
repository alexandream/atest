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

at_test(Success) {
	at_assert(success());
}


at_test(Failure) {
	at_assert(failure());
}


int test_data[] = { 1, 2, 3 };
AtArrayIterator iterwhatever = at_static_array_iterator(test_data);

at_data_driven_test(Errorx, iterwhatever, int, data) {
	at_assert(error());
}


int main(int argc, char** argv) {
	AtStreamReporter* reporter = NULL;
	AtSuite* suite = 
		at_new_suite("Strdup", NULL, NULL, setup, teardown);

	
	at_add_test(suite, &Success);
	at_add_test(suite, &Failure);
	at_add_test(suite, &Errorx);
	
	if (argc > 1) {
		const char* file_name = argv[1];
		reporter = at_new_stream_reporter(file_name);
	}
		
	at_run_suite(suite, (AtReporter*) reporter);
	return 0;
}
