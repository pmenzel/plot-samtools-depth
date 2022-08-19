CC = gcc
CFLAGS = -Wall -Wconversion -O3 -g -ansi -pedantic -std=gnu99
LDLIBS = -lplot -lm

all: plot-samtools-depth

%.o : %.c
	$(CC) $(CFLAGS)  -c -o $@ $<

plot-samtools-depth: plot-samtools-depth.o
	$(CC) $(LDFLAGS) -o plot-samtools-depth plot-samtools-depth.o $(LDLIBS)

clean:
	rm -f plot-samtools-depth
	find . -name "*.o" -delete
