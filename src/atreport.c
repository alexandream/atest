#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

/* This is an ugly, quick and dirty implementation of the report 
 * pretty-printer. I might revisit it later, but I needed the feature and
 * didn't have time to sink here. */

typedef struct SuiteSummary SuiteSummary;


#define EQUAL_40 "========================================"
#define DASH_40  "----------------------------------------"

#define EQUAL_80 (EQUAL_40 EQUAL_40)
#define DASH_80 (DASH_40 DASH_40)

struct SuiteSummary {
    const char* name;
    int tests;
    int errors;
    int failures;
};

int process_output_file(const char* file_name, SuiteSummary *summary);


int main(int argc, char** argv) {
    SuiteSummary* summary_pool;
    int num_summaries = 0;
    int max_summaries = argc -1;
    int error = 0;
    int i;

    if (argc < 2) {
        fprintf(stderr, "Wrong number of arguments.\n");
        exit(1);
    }

    summary_pool = malloc(sizeof(SuiteSummary)*max_summaries);
    if (summary_pool == NULL) {
        fprintf(stderr, "Allocation error.\n");
        exit(2);
    }
    
    for (i = 0; i < max_summaries; i++) {
        SuiteSummary *summary = summary_pool + i;
        const char* file_name = argv[i+1];
        error = process_output_file(file_name, summary);
        if (error) {
            fprintf(stderr, "Got error %d while processing file %s.\n",
                    error, file_name);
            exit(3);
        }
        num_summaries++;
    }

    printf ("%s\n", EQUAL_80);
    printf("| %-46s| %s | %s | %s | %s |\n",
           "Suite Name", "Tests", "Errors", "Fails", " %%");
    printf("|%s|%s|%s|%s|%s|\n",
           "-----------------------------------------------",
           "-------","--------","-------","-----");


    for (i = 0; i < num_summaries; i++) {
        SuiteSummary *summary = summary_pool +i;
        int percent = 100 - 100 * (summary->errors + summary->failures)
                                / summary->tests;
        printf("| %-46s|   %3d |    %3d |   %3d | %3d |\n",
               summary->name, summary->tests, summary->errors,
               summary->failures, percent);
    }
    printf(" %s \n",
           "---------------------------------------"
           "---------------------------------------");
}


long find_file_size(FILE* fp) {
    long file_size;
    if (fseek(fp, 0L, SEEK_END)) {
        return -1;
    }

    file_size = ftell(fp);
    if (file_size < 0) {
        return -2;
    }

    if (fseek(fp, 0L, SEEK_SET)) {
        return -3;
    }

    return file_size;
}


char* read_file_to_memory(const char* file_name) {
    FILE* fp = fopen(file_name, "r");
    char* buffer = NULL;
    long file_size;

    if (fp == NULL) {
        goto clean_up;
    }
    
    file_size = find_file_size(fp);
    if (file_size < 0) {
        goto clean_up;
    }

    buffer = malloc(file_size +1);
    if (buffer == NULL) {
        goto clean_up;
    }

    if (fread(buffer, 1, file_size, fp) != file_size) {
        goto clean_up;
    }
    buffer[file_size] = '\0';
    fclose(fp);
    return buffer;

clean_up:
    if (fp != NULL) {
        fclose(fp);
    }

    if (buffer != NULL) {
        free(buffer);
    }
    return NULL;
}

char* read_line(char** buffer) {
    char *result = *buffer;
    char* eol = strchr(*buffer, '\n');
    if (eol) {
        *eol = '\0';
        *buffer = eol+1;
    }
    else {
        size_t len = strlen(*buffer);
        *buffer = *buffer + len;
    }
    return result;
}


void process_header(const char* name, SuiteSummary* summary) {
    summary->name = name;
}


void process_footer(char* line, SuiteSummary* summary) {
    strtok(line, "\t");
    summary->tests = atoi(strtok(NULL, "\t"));
    summary->errors = atoi(strtok(NULL, "\t"));
    summary->failures = atoi(strtok(NULL, "\t"));
}


char*
decode_crlf(const char* input) {
	char* result;
	size_t length = strlen(input);
	size_t i;
	size_t j;

	result = malloc(sizeof(char) * (length + 1));
	if (result == NULL) {
		return NULL;
	}

	for (i = 0, j = 0; i < length; i++, j++) {
		if (input[i] == '\\') {
			char c = input[i+1];
			if (c != '\\' && c != 'n' && c != 'r') {
				/* Invalid codification. Free up memory and set errno */
				free(result);
				errno = EDOM;
				return NULL;
			}
			result[j] = (c == '\\') ? '\\'
			          : (c == 'r') ? '\r'
			          : '\n'; /* Can only be n */
			i++;
		}
		else {
			result[j] = input[i];
		}
	}
	result[j] = '\0';

	return result;
}


void process_result(char* line, const char* suite) {
    char extra_info[81];
    char dash_line[81];
    
    const char* type = strtok(line, "\t");
    const char* name = strtok(NULL, "\t");
    const char* file = strtok(NULL, "\t");

    char* line_num = strtok(NULL, "\t");
    char* check = strtok(NULL, "\t");
    char* n_checks = strtok(NULL, "\t");
    
    char* msg = decode_crlf(strtok(NULL, "\t"));
    int extra_info_len = strlen(file) + strlen(line_num) + strlen(check) +
                         strlen(n_checks) + 7;


    if (msg == NULL) { 
        fprintf(stderr, "Allocation error.\n");
        exit(19);
    }

    printf("%s\n", EQUAL_80);

    printf("%-5s: %s.%s\n",
           *type == 'E' ? "ERROR" : "FAIL",
           suite, name);

    strcpy(dash_line, DASH_80);
    sprintf(extra_info,  " %s, %s, %s/%s ",
            file, line_num, check, n_checks);
    memcpy(dash_line+(80 - extra_info_len ), extra_info, extra_info_len);
    printf("%s\n\n", dash_line);
    printf("%s\n\n", msg);
    free(msg);
}


int process_output_file(const char* file_name, SuiteSummary *summary) {
    char* buffer = read_file_to_memory(file_name);
    char* p = buffer;

    if (buffer == NULL) {
        return -1;
    }

    process_header(read_line(&p), summary);
    while(*p && *p != '#') {
        process_result(read_line(&p), summary->name);
    }
    process_footer(read_line(&p), summary);
    
    return 0;
}
