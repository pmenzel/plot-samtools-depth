# plot-samtools-depth

## About

This program plots the average sequencing depth summarized in windows along a chromosome using the output of [samtools depth](http://www.htslib.org/doc/samtools-depth.html), using [GNU libplot](https://www.gnu.org/software/plotutils/manual/en/html_node/libplot.html#libplot).
The output is a postscript file.

## Usage
Download:
```
sudo apt install libplot-dev

git clone https://github.com/pmenzel/plot-samtools-depth.git

make

samtools depth -s -aa /path/to/mapping.bam > mapping.depth

./plot-samtools-depth -i mapping.depth > depth-plot.ps

evince depth-plot.ps
```

## Example

![Example](/example.png?raw=true)
