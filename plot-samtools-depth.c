/*

	 Copyright (C) 2022 Peter Menzel

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
#define PROGRAMNAME "plot-samtools-depth"

void print_version(FILE *file)
{
	fprintf(file, PROGRAMNAME " " VERSION "\n");
	fprintf(file,"Copyright (C) 2022 by Peter Menzel\n");
}

void usage(char *progname) {
	fprintf(stderr, "Usage:\n  %s -i FILENAME\n", progname);
	fprintf(stderr, "\n");
	fprintf(stderr, "Mandatory arguments:\n");
	fprintf(stderr, "   -i FILENAME   Name of input file\n");
	fprintf(stderr, "   -t STRING     plotlib display type (ps)\n");
	fprintf(stderr, "   -w INT        window size (10000)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Optional arguments:\n");
	fprintf(stderr, "   -d            Enable debug output.\n");
	exit(EXIT_FAILURE);
}


int main(int argc, char **argv) {
	char *display_type = "ps";	/* default libplot output format 	*/
	char *inputfilename = NULL;
	unsigned long window_size = 10000;
	double tick_frac_x = 0.1; // fraction of number of windows for which to plot x-axis ticks and labels
	int debug = 0;
	char buffer [55]; /* text buffer for printing text */

	/* parse command line */
	unsigned long input_number_w = ULONG_MAX;
	opterr = 0;
	int optc;
	while((optc = getopt (argc, argv, "di:t:w:")) != -1) {
		switch(optc) {
			case 'i':
				inputfilename = optarg;
				break;
			case 't':
				display_type = optarg;
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

	if(!inputfilename) {
		printf("Missing input filename.\n");
		usage(argv[0]);
	}

	FILE * fp = fopen(inputfilename,"r");
	if(!fp) {
		fprintf(stderr,"Could not open file %s\n",inputfilename);
		usage(argv[0]);
	}



	int handle;			/* handle for open plotter		*/
	int return_value;			/* handle for open plotter		*/


	/* initialising one plot session	*/
	/* specify type of plotter		*/
	handle = pl_newpl(display_type, NULL, stdout, stderr);
	if(handle < 0) {
		fprintf(stderr,"The plotter could not be created.\n");
 		exit(1);
	}

	return_value = pl_selectpl(handle);
	if(return_value < 0) {
		fprintf(stderr, "The plotter does not exist or could not be selected.\n");
 		exit(1);
	}

	return_value = pl_openpl();
	if(return_value < 0) {
		fprintf(stderr, "The selected plotter could not be opened!\n");
		exit(1);
	}

	//	return_value = pl_fspace(-1.4,-1.4,1.4,1.4);
	//	if(return_value) {	fprintf(stderr,"fspace returned %d!\n",return_value);	}


	pl_joinmod("round");
	pl_filltype(1);
	pl_flinewidth(-1);
	pl_fillcolorname("black");

	int window_count = 0;

	// buffer for storing values for one window length
	u_int64_t * arr;
	arr = malloc(window_size * sizeof(u_int64_t));

	int curr_pos = 0;
	u_int64_t total_pos = 0;

	// store min and max y-values for making bounding box later
	double avg_depth_min = 100000.0;
	double avg_depth_max = 0.0;

	char *line = NULL;
	size_t size = 0;;
	ssize_t length_line;
	while((length_line = getline(&line, &size, fp)) != -1) {
		if(length_line == 1) { continue; } /* only newline char */

		char * firsttab = strchr(line,'\t');
		if(firsttab)  {
			char * secondtab = strchr(firsttab+1,'\t');
			if(secondtab)  {
				u_int64_t input_number = ULONG_MAX;
				input_number = strtoul(secondtab, NULL, 10);
				if(input_number == 0 || input_number == ULONG_MAX) { fprintf(stderr, "Bad number (out of range error) in input line: %s",line); exit(EXIT_FAILURE); }
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
					if(debug) fprintf(stderr, "avg at %li is %f\n", total_pos, avg);
					avg_depth_min = min(avg, avg_depth_min);
					avg_depth_max = max(avg, avg_depth_max);

					// actually not needed:
					memset(arr, 0, window_size * sizeof(u_int64_t));

					curr_pos = 0;
					window_count++;

					// draw dot
					pl_fcircle(0.01 * (double)window_count, 0.01 * avg, 0.001);

				}
			}
		}
	}
	free(line);
	fclose(fp);

	//TODO what about the last positions that didn't fill up a window

	free(arr);

	pl_filltype(0);
	pl_fbox(-0.01, 0.01 * (avg_depth_min - 1), 0.01 * (double)(window_count + 1), 0.01 * (avg_depth_max + 1));

	// label for min depth on y-axis
	pl_fmove(-0.02, 0.01 * avg_depth_min);
	sprintf(buffer, "%.0lf", avg_depth_min);
	pl_alabel('r', 'c', buffer);

	// label for max depth on y-axis
	pl_fmove(-0.02, 0.01 * avg_depth_max);
	sprintf(buffer, "%.0lf", avg_depth_max);
	pl_alabel('r', 'c', buffer);

	// draw x-axis labels
	for(int i = 0; i < window_count; i += (double)window_count * tick_frac_x) {
		pl_fmove(0.01 * i, 0.01 * (avg_depth_min - 1));
		sprintf(buffer, "%li", i * window_size / 1000);
		pl_alabel('c', 't', buffer);
	}
	// draw x-axis title
	pl_fmove(0.01 * (double)window_count / 2.0, 0.01 * (avg_depth_min - 5));
	pl_alabel('c', 't', "Genome location (Kbp)");

	/* end a plot sesssion			*/
	return_value = pl_closepl();
	if(return_value < 0) {
		fprintf(stderr,"The plotter could not be closed.\n");
		/* no exit, because we try to delete the plotter 		*/
	}

	/* need to select a different plotter in order to delete our	*/
	return_value = pl_selectpl(0);
	if(return_value < 0) {
		fprintf(stderr, "Default Plotter could not be selected!\n");
	}

	return_value = pl_deletepl(handle);/* clean up by deleting used plotter	*/
	if(return_value < 0) {
		fprintf(stderr,"Selected Plotter could not be deleted!\n");
	}

	return 0;
}


