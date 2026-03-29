#include <stdio.h>
#include <string.h>
#include "gzip_io.h"

static int ends_with_gz(const char *path) {
    size_t len = strlen(path);

    if (len < 3) {
        return 0;
    }

    return strcmp(path + len - 3, ".gz") == 0;
}

int open_input_file(const char *path, InputFile *infile) {
    infile->fp = NULL;
    infile->gzfp = NULL;

    if (ends_with_gz(path)) {
        infile->type = INPUT_GZIP;
        infile->gzfp = gzopen(path, "rb");

        if (infile->gzfp == NULL) {
            fprintf(stderr, "ERROR: cannot open gzip file: %s\n", path);
            return 1;
        }

        return 0;
    }

    infile->type = INPUT_PLAIN;
    infile->fp = fopen(path, "r");

    if (infile->fp == NULL) {
        fprintf(stderr, "ERROR: cannot open plain text file: %s\n", path);
        return 1;
    }

    return 0;
}

int close_input_file(InputFile *infile) {
    if (infile->type == INPUT_GZIP) {
        if (infile->gzfp != NULL) {
            if (gzclose(infile->gzfp) != Z_OK) {
                fprintf(stderr, "ERROR: failed to close gzip file\n");
                return 1;
            }
            infile->gzfp = NULL;
        }
        return 0;
    }

    if (infile->fp != NULL) {
        if (fclose(infile->fp) != 0) {
            fprintf(stderr, "ERROR: failed to close plain text file\n");
            return 1;
        }
        infile->fp = NULL;
    }

    return 0;
}
