#ifndef FASTQ_READER_H
#define FASTQ_READER_H

#include "gzip_io.h"

#define FASTQ_MAX_LINE 65536

typedef struct {
    char header[FASTQ_MAX_LINE];
    char sequence[FASTQ_MAX_LINE];
    char plus[FASTQ_MAX_LINE];
    char quality[FASTQ_MAX_LINE];
} FastqRecord;

int read_fastq_record(InputFile *infile, FastqRecord *record, int *eof_reached);

#endif
