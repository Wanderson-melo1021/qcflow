#ifndef GZIP_IO_H
#define GZIP_IO_H

#include <stdio.h>
#include <zlib.h>

typedef enum {
    INPUT_PLAIN,
    INPUT_GZIP
} InputType;

typedef struct {
    InputType type;
    FILE *fp;
    gzFile gzfp;
} InputFile;

int open_input_file(const char *path, InputFile *infile);
int close_input_file(InputFile *infile);

#endif
