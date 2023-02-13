# plot-samtools-depth

## About

This program plots the average sequencing depth summarized in windows along a chromosome using the output of [samtools depth](http://www.htslib.org/doc/samtools-depth.html), using [GNU libplot](https://www.gnu.org/software/plotutils/manual/en/html_node/libplot.html#libplot).
The output is a postscript file.

## Usage
Download:
```
sudo apt install libplot-dev

git clone https://github.com/pmenzel/plot-samtools-depth.git

cd plot-samtools-depth && make

samtools depth -s -aa /path/to/mapping.bam > mapping.depth

./plot-samtools-depth -i mapping.depth > depth-plot.ps

evince depth-plot.ps
```

## Example

![Example](/example.png?raw=true)

# bin-samtools-depth

## About

This calculates the average depth in bins along a chromosome using the output of [samtools depth](http://www.htslib.org/doc/samtools-depth.html).

## Usage
Download:
```
samtools depth -s -aa /path/to/mapping.bam > mapping.depth

./bin-samtools-depth -i mapping.depth > binned_depths.tsv
```

The output has 3 columns in TSV format, for example:

```
contig_1  50000   93.145200
contig_2  50000   72.055800
contig_2  100000  68.510700
contig_2  150000  67.599680
contig_2  200000  68.697940
```

