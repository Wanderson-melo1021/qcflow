#ifndef CLI_H
#define CLI_H

typedef struct {
    char *input1;
    char *input2;
    char *output_prefix;
    int paired_end;

    int generate_report;
    char *report_script;
} QcConfig;

int run_cli(int argc, char *argv[]);

#endif
