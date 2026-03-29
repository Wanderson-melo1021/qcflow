#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cli.h"
#include "gzip_io.h"
#include "fastq_reader.h"
#include "qc_stats.h"
#include "writer.h"

static void print_general_usage(void) {
    fprintf(stderr, "Usage: qcflow <command> [options]\n");
    fprintf(stderr, "Available commands: qc\n");
}

static void print_qc_usage(void) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  qcflow qc --in <file.fastq.gz> --out <prefix>\n");
    fprintf(stderr, "  qcflow qc --in1 <R1.fastq.gz> --in2 <R2.fastq.gz> --out <prefix>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Optional:\n");
    fprintf(stderr, "  --report --report-script <path>\n");
}

static int parse_qc_args(int argc, char *argv[], QcConfig *config) {
    config->input1 = NULL;
    config->input2 = NULL;
    config->output_prefix = NULL;
    config->paired_end = 0;
    config->generate_report = 0;
    config->report_script = NULL;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--in") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: --in requires a value\n");
                return 1;
            }
            config->input1 = argv[++i];
        } else if (strcmp(argv[i], "--in1") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: --in1 requires a value\n");
                return 1;
            }
            config->input1 = argv[++i];
        } else if (strcmp(argv[i], "--in2") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: --in2 requires a value\n");
                return 1;
            }
            config->input2 = argv[++i];
        } else if (strcmp(argv[i], "--out") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: --out requires a value\n");
                return 1;
            }
            config->output_prefix = argv[++i];
        } else if (strcmp(argv[i], "--report") == 0) {
            config->generate_report = 1;
        } else if (strcmp(argv[i], "--report-script") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: --report-script requires a value\n");
                return 1;
            }
            config->report_script = argv[++i];
        } else {
            fprintf(stderr, "ERROR: unknown option for qc: %s\n", argv[i]);
            return 1;
        }
    }

    if (config->output_prefix == NULL) {
        fprintf(stderr, "ERROR: --out is required\n");
        return 1;
    }

    if (config->input1 != NULL && config->input2 == NULL) {
        config->paired_end = 0;
        return 0;
    }

    if (config->input1 != NULL && config->input2 != NULL) {
        config->paired_end = 1;
        return 0;
    }

    if (config->generate_report && config->report_script == NULL) {
        fprintf(stderr, "ERROR: --report requires --report-script <path>\n");
        return 1;
    }

    fprintf(stderr, "ERROR: invalid input arguments\n");
    return 1;
}

static int collect_qc_stats(const char *path, QcStats *stats) {
    InputFile infile;
    FastqRecord record;
    int eof_reached = 0;

    qc_stats_init(stats);

    if (open_input_file(path, &infile) != 0) {
        return 1;
    }

    while (!eof_reached) {
        int status = read_fastq_record(&infile, &record, &eof_reached);

        if (status != 0) {
            close_input_file(&infile);
            qc_stats_free(stats);
            return 1;
        }

        if (!eof_reached) {
            if (qc_stats_update(stats, &record) != 0) {
                fprintf(stderr, "ERROR: failed to update QC statistics\n");
                close_input_file(&infile);
                qc_stats_free(stats);
                return 1;
            }
        }
    }

    if (close_input_file(&infile) != 0) {
        qc_stats_free(stats);
        return 1;
    }

    return 0;
}

static void print_qc_stats(const char *label, const QcStats *stats) {
    printf("  %s total_reads: %ld\n", label, stats->total_reads);
    printf("  %s total_bases: %ld\n", label, stats->total_bases);
    printf("  %s min_length: %ld\n", label, stats->min_length);
    printf("  %s max_length: %ld\n", label, stats->max_length);
    printf("  %s mean_length: %.2f\n", label, qc_stats_mean_length(stats));
    printf("  %s gc_bases: %ld\n", label, stats->gc_bases);
    printf("  %s n_bases: %ld\n", label, stats->n_bases);
    printf("  %s gc_percent: %.2f\n", label, qc_stats_gc_percent(stats));
    printf("  %s n_percent: %.2f\n", label, qc_stats_n_percent(stats));
    printf("  %s mean_quality: %.2f\n", label, qc_stats_mean_quality(stats));
}

static int write_qc_outputs_se(const char *prefix, const QcStats *stats) {
    if (write_summary_tsv_se(prefix, stats) != 0) {
        return 1;
    }
    if (write_per_base_quality_tsv_se(prefix, stats) != 0) {
        return 1;
    }
    if (write_per_base_gc_tsv_se(prefix, stats) != 0) {
        return 1;
    }
    if (write_per_base_n_tsv_se(prefix, stats) != 0) {
        return 1;
    }
    if (write_length_distribution_tsv_se(prefix, stats) != 0) {
        return 1;
    }

    return 0;
}

static int write_qc_outputs_pe(const char *prefix, const QcStats *stats1, const QcStats *stats2) {
    if (write_summary_tsv_pe(prefix, stats1, stats2) != 0) {
        return 1;
    }
    if (write_per_base_quality_tsv_pe(prefix, stats1, stats2) != 0) {
        return 1;
    }
    if (write_per_base_gc_tsv_pe(prefix, stats1, stats2) != 0) {
        return 1;
    }
    if (write_per_base_n_tsv_pe(prefix, stats1, stats2) != 0) {
        return 1;
    }
    if (write_length_distribution_tsv_pe(prefix, stats1, stats2) != 0) {
        return 1;
    }

    return 0;
}

static int run_report(const char *report_script, const char *output_prefix) {
    char cmd[8192];
    int written;
    int status;

    written = snprintf(
        cmd,
        sizeof(cmd),
        "Rscript \"%s\" \"%s\"",
        report_script,
        output_prefix
    );

    if (written < 0 || (size_t) written >= sizeof(cmd)) {
        fprintf(stderr, "ERROR: report command too long\n");
        return 1;
    }

    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "ERROR: failed to generate pre-report with Rscript\n");
        return 1;
    }

    return 0;
}

static int run_qc_command(int argc, char *argv[]) {
    QcConfig config;
    QcStats stats1;
    QcStats stats2;

    if (parse_qc_args(argc, argv, &config) != 0) {
        print_qc_usage();
        return 1;
    }

    printf("qcflow qc configuration\n");
    printf("  input1: %s\n", config.input1);
    printf("  input2: %s\n", config.input2 ? config.input2 : "NULL");
    printf("  out: %s\n", config.output_prefix);
    printf("  mode: %s\n", config.paired_end ? "paired-end" : "single-end");
    printf("  report: %s\n", config.generate_report ? "true" : "false");
    printf("  report_script: %s\n", config.report_script ? config.report_script : "NULL");

    if (collect_qc_stats(config.input1, &stats1) != 0) {
        return 1;
    }

    print_qc_stats("input1", &stats1);

    if (!config.paired_end) {
        if (write_qc_outputs_se(config.output_prefix, &stats1) != 0) {
            qc_stats_free(&stats1);
            return 1;
        }

        if (config.generate_report) {
            if (run_report(config.report_script, config.output_prefix) != 0) {
                qc_stats_free(&stats1);
                return 1;
            }
        }

        qc_stats_free(&stats1);
        return 0;
    }

    if (collect_qc_stats(config.input2, &stats2) != 0) {
        qc_stats_free(&stats1);
        return 1;
    }

    print_qc_stats("input2", &stats2);

    if (stats1.total_reads != stats2.total_reads) {
        fprintf(stderr, "ERROR: paired-end files have different numbers of reads\n");
        qc_stats_free(&stats1);
        qc_stats_free(&stats2);
        return 1;
    }

    if (write_qc_outputs_pe(config.output_prefix, &stats1, &stats2) != 0) {
        qc_stats_free(&stats1);
        qc_stats_free(&stats2);
        return 1;
    }

    if (config.generate_report) {
        if (run_report(config.report_script, config.output_prefix) != 0) {
            qc_stats_free(&stats1);
            qc_stats_free(&stats2);
            return 1;
        }
    }

    qc_stats_free(&stats1);
    qc_stats_free(&stats2);
    return 0;
}

int run_cli(int argc, char *argv[]) {
    if (argc < 2) {
        print_general_usage();
        return 1;
    }

    if (strcmp(argv[1], "qc") == 0) {
        return run_qc_command(argc, argv);
    }

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    print_general_usage();
    return 1;
}
