CC=gcc -Wall -Wconversion -O3 -g -ansi -pedantic -std=gnu99

all: plot-samtools-depth

plot-samtools-depth: plot-samtools-depth.c
	$(CC) $(CFLAGS) $(LDFLAGS) plot-samtools-depth.c -o $@ -lplot -lm

