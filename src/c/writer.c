#include <stdio.h>
#include <string.h>
#include "writer.h"

static int make_path(char *buffer, size_t size, const char *prefix, const char *suffix) {
    int written = snprintf(buffer, size, "%s%s", prefix, suffix);

    if (written < 0 || (size_t) written >= size) {
        fprintf(stderr, "ERROR: output path too long\n");
        return 1;
    }

    return 0;
}

/* =========================
 * Single-end writers
 * ========================= */

int write_summary_tsv_se(const char *output_prefix, const QcStats *stats) {
    char path[4096];
    FILE *fp;

    if (make_path(path, sizeof(path), output_prefix, ".summary.tsv") != 0) {
        return 1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: cannot write summary file: %s\n", path);
        return 1;
    }

    fprintf(fp, "metric\tscope\tvalue\n");
    fprintf(fp, "paired_end\tsample\tfalse\n");
    fprintf(fp, "total_reads\tsample\t%ld\n", stats->total_reads);
    fprintf(fp, "total_bases\tsample\t%ld\n", stats->total_bases);
    fprintf(fp, "min_length\tsample\t%ld\n", stats->min_length);
    fprintf(fp, "max_length\tsample\t%ld\n", stats->max_length);
    fprintf(fp, "mean_length\tsample\t%.2f\n", qc_stats_mean_length(stats));
    fprintf(fp, "gc_bases\tsample\t%ld\n", stats->gc_bases);
    fprintf(fp, "n_bases\tsample\t%ld\n", stats->n_bases);
    fprintf(fp, "gc_percent\tsample\t%.2f\n", qc_stats_gc_percent(stats));
    fprintf(fp, "n_percent\tsample\t%.2f\n", qc_stats_n_percent(stats));
    fprintf(fp, "mean_quality\tsample\t%.2f\n", qc_stats_mean_quality(stats));

    fclose(fp);
    return 0;
}

int write_per_base_quality_tsv_se(const char *output_prefix, const QcStats *stats) {
    char path[4096];
    FILE *fp;

    if (make_path(path, sizeof(path), output_prefix, ".per_base_quality.tsv") != 0) {
        return 1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: cannot write per-base quality file: %s\n", path);
        return 1;
    }

    fprintf(fp, "cycle\tmate\tmean_quality\tcount\n");

    for (long cycle = 1; cycle <= stats->per_base_capacity; cycle++) {
        long idx = cycle - 1;
        if (stats->per_base_counts[idx] == 0) {
            continue;
        }

        fprintf(fp, "%ld\tSE\t%.2f\t%ld\n",
                cycle,
                qc_stats_mean_quality_at_cycle(stats, cycle),
                stats->per_base_counts[idx]);
    }

    fclose(fp);
    return 0;
}

int write_per_base_gc_tsv_se(const char *output_prefix, const QcStats *stats) {
    char path[4096];
    FILE *fp;

    if (make_path(path, sizeof(path), output_prefix, ".per_base_gc.tsv") != 0) {
        return 1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: cannot write per-base GC file: %s\n", path);
        return 1;
    }

    fprintf(fp, "cycle\tmate\tgc_percent\tcount\n");

    for (long cycle = 1; cycle <= stats->per_base_capacity; cycle++) {
        long idx = cycle - 1;
        if (stats->per_base_counts[idx] == 0) {
            continue;
        }

        fprintf(fp, "%ld\tSE\t%.2f\t%ld\n",
                cycle,
                qc_stats_gc_percent_at_cycle(stats, cycle),
                stats->per_base_counts[idx]);
    }

    fclose(fp);
    return 0;
}

int write_per_base_n_tsv_se(const char *output_prefix, const QcStats *stats) {
    char path[4096];
    FILE *fp;

    if (make_path(path, sizeof(path), output_prefix, ".per_base_n.tsv") != 0) {
        return 1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: cannot write per-base N file: %s\n", path);
        return 1;
    }

    fprintf(fp, "cycle\tmate\tn_percent\tcount\n");

    for (long cycle = 1; cycle <= stats->per_base_capacity; cycle++) {
        long idx = cycle - 1;
        if (stats->per_base_counts[idx] == 0) {
            continue;
        }

        fprintf(fp, "%ld\tSE\t%.2f\t%ld\n",
                cycle,
                qc_stats_n_percent_at_cycle(stats, cycle),
                stats->per_base_counts[idx]);
    }

    fclose(fp);
    return 0;
}

int write_length_distribution_tsv_se(const char *output_prefix, const QcStats *stats) {
    char path[4096];
    FILE *fp;

    if (make_path(path, sizeof(path), output_prefix, ".length_distribution.tsv") != 0) {
        return 1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: cannot write length distribution file: %s\n", path);
        return 1;
    }

    fprintf(fp, "length\tmate\tcount\n");

    for (long len = 0; len < stats->length_hist_capacity; len++) {
        if (stats->length_hist[len] == 0) {
            continue;
        }

        fprintf(fp, "%ld\tSE\t%ld\n", len, stats->length_hist[len]);
    }

    fclose(fp);
    return 0;
}

/* =========================
 * Paired-end writers
 * ========================= */

int write_summary_tsv_pe(const char *output_prefix,
                         const QcStats *stats1,
                         const QcStats *stats2) {
    char path[4096];
    FILE *fp;
    int read_count_match = (stats1->total_reads == stats2->total_reads);

    if (make_path(path, sizeof(path), output_prefix, ".summary.tsv") != 0) {
        return 1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: cannot write summary file: %s\n", path);
        return 1;
    }

    fprintf(fp, "metric\tscope\tvalue\n");
    fprintf(fp, "paired_end\tpair\ttrue\n");
    fprintf(fp, "read_count_match\tpair\t%s\n", read_count_match ? "true" : "false");

    fprintf(fp, "total_reads\tR1\t%ld\n", stats1->total_reads);
    fprintf(fp, "total_reads\tR2\t%ld\n", stats2->total_reads);

    fprintf(fp, "total_bases\tR1\t%ld\n", stats1->total_bases);
    fprintf(fp, "total_bases\tR2\t%ld\n", stats2->total_bases);

    fprintf(fp, "min_length\tR1\t%ld\n", stats1->min_length);
    fprintf(fp, "min_length\tR2\t%ld\n", stats2->min_length);

    fprintf(fp, "max_length\tR1\t%ld\n", stats1->max_length);
    fprintf(fp, "max_length\tR2\t%ld\n", stats2->max_length);

    fprintf(fp, "mean_length\tR1\t%.2f\n", qc_stats_mean_length(stats1));
    fprintf(fp, "mean_length\tR2\t%.2f\n", qc_stats_mean_length(stats2));

    fprintf(fp, "gc_bases\tR1\t%ld\n", stats1->gc_bases);
    fprintf(fp, "gc_bases\tR2\t%ld\n", stats2->gc_bases);

    fprintf(fp, "n_bases\tR1\t%ld\n", stats1->n_bases);
    fprintf(fp, "n_bases\tR2\t%ld\n", stats2->n_bases);

    fprintf(fp, "gc_percent\tR1\t%.2f\n", qc_stats_gc_percent(stats1));
    fprintf(fp, "gc_percent\tR2\t%.2f\n", qc_stats_gc_percent(stats2));

    fprintf(fp, "n_percent\tR1\t%.2f\n", qc_stats_n_percent(stats1));
    fprintf(fp, "n_percent\tR2\t%.2f\n", qc_stats_n_percent(stats2));

    fprintf(fp, "mean_quality\tR1\t%.2f\n", qc_stats_mean_quality(stats1));
    fprintf(fp, "mean_quality\tR2\t%.2f\n", qc_stats_mean_quality(stats2));

    fclose(fp);
    return 0;
}

int write_per_base_quality_tsv_pe(const char *output_prefix,
                                  const QcStats *stats1,
                                  const QcStats *stats2) {
    char path[4096];
    FILE *fp;
    long max_cycle = stats1->per_base_capacity > stats2->per_base_capacity
                   ? stats1->per_base_capacity : stats2->per_base_capacity;

    if (make_path(path, sizeof(path), output_prefix, ".per_base_quality.tsv") != 0) {
        return 1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: cannot write per-base quality file: %s\n", path);
        return 1;
    }

    fprintf(fp, "cycle\tmate\tmean_quality\tcount\n");

    for (long cycle = 1; cycle <= max_cycle; cycle++) {
        long idx = cycle - 1;

        if (cycle <= stats1->per_base_capacity && stats1->per_base_counts[idx] > 0) {
            fprintf(fp, "%ld\tR1\t%.2f\t%ld\n",
                    cycle,
                    qc_stats_mean_quality_at_cycle(stats1, cycle),
                    stats1->per_base_counts[idx]);
        }

        if (cycle <= stats2->per_base_capacity && stats2->per_base_counts[idx] > 0) {
            fprintf(fp, "%ld\tR2\t%.2f\t%ld\n",
                    cycle,
                    qc_stats_mean_quality_at_cycle(stats2, cycle),
                    stats2->per_base_counts[idx]);
        }
    }

    fclose(fp);
    return 0;
}

int write_per_base_gc_tsv_pe(const char *output_prefix,
                             const QcStats *stats1,
                             const QcStats *stats2) {
    char path[4096];
    FILE *fp;
    long max_cycle = stats1->per_base_capacity > stats2->per_base_capacity
                   ? stats1->per_base_capacity : stats2->per_base_capacity;

    if (make_path(path, sizeof(path), output_prefix, ".per_base_gc.tsv") != 0) {
        return 1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: cannot write per-base GC file: %s\n", path);
        return 1;
    }

    fprintf(fp, "cycle\tmate\tgc_percent\tcount\n");

    for (long cycle = 1; cycle <= max_cycle; cycle++) {
        long idx = cycle - 1;

        if (cycle <= stats1->per_base_capacity && stats1->per_base_counts[idx] > 0) {
            fprintf(fp, "%ld\tR1\t%.2f\t%ld\n",
                    cycle,
                    qc_stats_gc_percent_at_cycle(stats1, cycle),
                    stats1->per_base_counts[idx]);
        }

        if (cycle <= stats2->per_base_capacity && stats2->per_base_counts[idx] > 0) {
            fprintf(fp, "%ld\tR2\t%.2f\t%ld\n",
                    cycle,
                    qc_stats_gc_percent_at_cycle(stats2, cycle),
                    stats2->per_base_counts[idx]);
        }
    }

    fclose(fp);
    return 0;
}

int write_per_base_n_tsv_pe(const char *output_prefix,
                            const QcStats *stats1,
                            const QcStats *stats2) {
    char path[4096];
    FILE *fp;
    long max_cycle = stats1->per_base_capacity > stats2->per_base_capacity
                   ? stats1->per_base_capacity : stats2->per_base_capacity;

    if (make_path(path, sizeof(path), output_prefix, ".per_base_n.tsv") != 0) {
        return 1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: cannot write per-base N file: %s\n", path);
        return 1;
    }

    fprintf(fp, "cycle\tmate\tn_percent\tcount\n");

    for (long cycle = 1; cycle <= max_cycle; cycle++) {
        long idx = cycle - 1;

        if (cycle <= stats1->per_base_capacity && stats1->per_base_counts[idx] > 0) {
            fprintf(fp, "%ld\tR1\t%.2f\t%ld\n",
                    cycle,
                    qc_stats_n_percent_at_cycle(stats1, cycle),
                    stats1->per_base_counts[idx]);
        }

        if (cycle <= stats2->per_base_capacity && stats2->per_base_counts[idx] > 0) {
            fprintf(fp, "%ld\tR2\t%.2f\t%ld\n",
                    cycle,
                    qc_stats_n_percent_at_cycle(stats2, cycle),
                    stats2->per_base_counts[idx]);
        }
    }

    fclose(fp);
    return 0;
}

int write_length_distribution_tsv_pe(const char *output_prefix,
                                     const QcStats *stats1,
                                     const QcStats *stats2) {
    char path[4096];
    FILE *fp;
    long max_len = stats1->length_hist_capacity > stats2->length_hist_capacity
                 ? stats1->length_hist_capacity : stats2->length_hist_capacity;

    if (make_path(path, sizeof(path), output_prefix, ".length_distribution.tsv") != 0) {
        return 1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: cannot write length distribution file: %s\n", path);
        return 1;
    }

    fprintf(fp, "length\tmate\tcount\n");

    for (long len = 0; len < max_len; len++) {
        if (len < stats1->length_hist_capacity && stats1->length_hist[len] > 0) {
            fprintf(fp, "%ld\tR1\t%ld\n", len, stats1->length_hist[len]);
        }

        if (len < stats2->length_hist_capacity && stats2->length_hist[len] > 0) {
            fprintf(fp, "%ld\tR2\t%ld\n", len, stats2->length_hist[len]);
        }
    }

    fclose(fp);
    return 0;
}
