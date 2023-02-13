/*

	 Copyright (C) 2022-2023 Peter Menzel

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <getopt.h>
#include <plot.h>
#include <string.h>

#define min(X, Y)  ((X) < (Y) ? (X) : (Y))
#define max(X, Y)  ((X) > (Y) ? (X) : (Y))


#define VERSION "0.1 "
#define PROGRAMNAME "bin-samtools-depth"

void print_version(FILE *file)
{
	fprintf(file, PROGRAMNAME " " VERSION "\n");
	fprintf(file,"Copyright (C) 2022-2023 by Peter Menzel\n");
}

void usage(char *progname) {
	fprintf(stderr, "Usage:\n  %s -i FILENAME\n", progname);
	fprintf(stderr, "\n");
	fprintf(stderr, "Mandatory arguments:\n");
	fprintf(stderr, "   -i FILENAME   Name of input file\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Optional arguments:\n");
	fprintf(stderr, "   -n STRING     only use this sequence name\n");
	fprintf(stderr, "   -w INT        window size (10000)\n");
	fprintf(stderr, "   -d            Enable debug output.\n");
	exit(EXIT_FAILURE);
}


int main(int argc, char **argv) {
	char *seq_name = NULL;
	char *inputfilename = NULL;
	unsigned long window_size = 10000;
	int debug = 0;
	int exit_code = 0;

	/* parse command line */
	unsigned long input_number_w = ULONG_MAX;
	opterr = 0;
	int optc;
	while((optc = getopt (argc, argv, "dn:i:w:")) != -1) {
		switch(optc) {
			case 'i':
				inputfilename = optarg;
				break;
			case 'n':
				seq_name = optarg;
				break;
			case 'w':
				input_number_w = strtoul(optarg, NULL, 10);
				if(input_number_w == 0 || input_number_w == ULONG_MAX) {
					fprintf(stderr, "Bad number (out of range error) in: %s", optarg);
					exit(EXIT_FAILURE);
				}
				window_size = input_number_w;
				break;
			case 'd':
				debug = 1;
				break;
			default:
				usage(argv[0]);
		}
	}

	if(seq_name != NULL) fprintf(stderr, "Only including sequence %s\n", seq_name);

	if(!inputfilename) {
		printf("Missing input filename.\n");
		usage(argv[0]);
	}

	FILE * fp = fopen(inputfilename,"r");
	if(!fp) {
		fprintf(stderr,"Could not open file %s\n",inputfilename);
		usage(argv[0]);
	}

	int window_count = 0;

	// buffer for storing values for one window length
	u_int64_t * arr;
	arr = malloc(window_size * sizeof(u_int64_t));

	int curr_pos = 0;
	u_int64_t total_pos = 0;

	char *line = NULL;
	size_t size = 0;;
	ssize_t length_line;
	char * curr_seq_name = NULL;
	while((length_line = getline(&line, &size, fp)) != -1) {
		if(length_line == 1) { continue; } /* only newline char */

		char * firsttab = strchr(line,'\t');
		if(firsttab)  {
			char * secondtab = strchr(firsttab+1,'\t');
			if(secondtab)  {

				// test if this sequence name should be included
				if(seq_name != NULL && strncmp(seq_name, line, strlen(seq_name)) != 0) { continue; }

				if(curr_seq_name == NULL || strncmp(curr_seq_name, line, strlen(curr_seq_name)) != 0) {
					if(curr_seq_name != NULL)
						free(curr_seq_name);
					int size =  firsttab - line + 1;
					curr_seq_name = malloc(size * sizeof(char));
					strncpy(curr_seq_name, line, size -1);
					total_pos = 0;
					curr_pos = 0;
					window_count = 0;
				}

				u_int64_t input_number = ULONG_MAX;
				input_number = strtoul(secondtab, NULL, 10);
				if(input_number < 0 || input_number == ULONG_MAX) {
					fprintf(stderr, "Bad number (out of range error) in input line: %s",line);
					exit_code = 1;
					goto theend;
				}
				//fprintf(stderr, "Num = %li\n", input_number);
				total_pos++;
				curr_pos++;
				arr[curr_pos] = input_number;

				if(curr_pos == window_size) {
					// calculate average over all numbers in arr
					u_int64_t sum = 0;
					for(int i = 0; i < window_size; i++) {
						sum += arr[i];
					}

					double avg = (double)sum / (double)window_size;
					printf("%.*s\t%li\t%f\n", firsttab-line, line, total_pos, avg);

					curr_pos = 0;
					window_count++;

				}
			}
		}
	}
	free(line);
	fclose(fp);

	// TODO what about the last positions that didn't fill up a window
	// ignore it!

	if(window_count <= 1) {
		fprintf(stderr, "Choose a smaller window size");
		exit_code = 1;
		goto theend;
	}

theend:

	free(arr);

	return exit_code;
}


