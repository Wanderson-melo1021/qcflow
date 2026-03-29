#include <stdio.h>
#include <string.h>
#include "fastq_reader.h"

static int read_line(InputFile *infile, char *buffer, int size) {
    char *result = NULL;

    if (infile->type == INPUT_GZIP) {
        result = gzgets(infile->gzfp, buffer, size);
    } else {
        result = fgets(buffer, size, infile->fp);
    }

    if (result == NULL) {
        return 0;
    }

    buffer[strcspn(buffer, "\r\n")] = '\0';
    return 1;
}

int read_fastq_record(InputFile *infile, FastqRecord *record, int *eof_reached) {
    int ok;

    *eof_reached = 0;

    ok = read_line(infile, record->header, FASTQ_MAX_LINE);
    if (!ok) {
        *eof_reached = 1;
        return 0;
    }

    if (!read_line(infile, record->sequence, FASTQ_MAX_LINE)) {
        fprintf(stderr, "ERROR: truncated FASTQ record after header: %s\n", record->header);
        return 1;
    }

    if (!read_line(infile, record->plus, FASTQ_MAX_LINE)) {
        fprintf(stderr, "ERROR: truncated FASTQ record after sequence: %s\n", record->header);
        return 1;
    }

    if (!read_line(infile, record->quality, FASTQ_MAX_LINE)) {
        fprintf(stderr, "ERROR: truncated FASTQ record after plus line: %s\n", record->header);
        return 1;
    }

    if (record->header[0] != '@') {
        fprintf(stderr, "ERROR: invalid FASTQ header line: %s\n", record->header);
        return 1;
    }

    if (record->plus[0] != '+') {
        fprintf(stderr, "ERROR: invalid FASTQ plus line for record: %s\n", record->header);
        return 1;
    }

    if (strlen(record->sequence) != strlen(record->quality)) {
        fprintf(stderr,
                "ERROR: sequence and quality length mismatch for record: %s\n",
                record->header);
        return 1;
    }

    return 0;
}
