#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "qc_stats.h"

static int ensure_per_base_capacity(QcStats *stats, long required_len) {
    if (required_len <= stats->per_base_capacity) {
        return 0;
    }

    long new_capacity = stats->per_base_capacity == 0 ? 128 : stats->per_base_capacity;
    while (new_capacity < required_len) {
        new_capacity *= 2;
    }

    long *new_quality_sum = realloc(stats->per_base_quality_sum, new_capacity * sizeof(long));
    if (new_quality_sum == NULL) {
        return 1;
    }

    stats->per_base_quality_sum = new_quality_sum;

    long *new_counts = realloc(stats->per_base_counts, new_capacity * sizeof(long));
    if (new_counts == NULL) {
        return 1;
    }

    stats->per_base_counts = new_counts;

    long *new_gc_counts = realloc(stats->per_base_gc_counts, new_capacity * sizeof(long));
    if (new_gc_counts == NULL) {
        return 1;
    }

    stats->per_base_gc_counts = new_gc_counts;

    long *new_n_counts = realloc(stats->per_base_n_counts, new_capacity * sizeof(long));
    if (new_n_counts == NULL) {
        return 1;
    }

    stats->per_base_n_counts = new_n_counts;

    for (long i = stats->per_base_capacity; i < new_capacity; i++) {
        stats->per_base_quality_sum[i] = 0;
        stats->per_base_counts[i] = 0;
        stats->per_base_gc_counts[i] = 0;
        stats->per_base_n_counts[i] = 0;
    }

    stats->per_base_capacity = new_capacity;
    return 0;
}

static int ensure_length_hist_capacity(QcStats *stats, long required_len) {
    if (required_len < stats->length_hist_capacity) {
        return 0;
    }

    long new_capacity = stats->length_hist_capacity == 0 ? 128 : stats->length_hist_capacity;
    while (new_capacity <= required_len) {
        new_capacity *= 2;
    }

    long *new_hist = realloc(stats->length_hist, new_capacity * sizeof(long));
    if (new_hist == NULL) {
        return 1;
    }

    for (long i = stats->length_hist_capacity; i < new_capacity; i++) {
        new_hist[i] = 0;
    }

    stats->length_hist = new_hist;
    stats->length_hist_capacity = new_capacity;
    return 0;
}

void qc_stats_init(QcStats *stats) {
    stats->total_reads = 0;
    stats->total_bases = 0;
    stats->min_length = -1;
    stats->max_length = 0;
    stats->gc_bases = 0;
    stats->n_bases = 0;
    stats->total_quality = 0;

    stats->per_base_quality_sum = NULL;
    stats->per_base_counts = NULL;
    stats->per_base_gc_counts = NULL;
    stats->per_base_n_counts = NULL;
    stats->per_base_capacity = 0;

    stats->length_hist = NULL;
    stats->length_hist_capacity = 0;
}

void qc_stats_free(QcStats *stats) {
    free(stats->per_base_quality_sum);
    free(stats->per_base_counts);
    free(stats->per_base_gc_counts);
    free(stats->per_base_n_counts);
    free(stats->length_hist);

    stats->per_base_quality_sum = NULL;
    stats->per_base_counts = NULL;
    stats->per_base_gc_counts = NULL;
    stats->per_base_n_counts = NULL;
    stats->length_hist = NULL;

    stats->per_base_capacity = 0;
    stats->length_hist_capacity = 0;
}

int qc_stats_update(QcStats *stats, const FastqRecord *record) {
    long len = (long) strlen(record->sequence);

    stats->total_reads++;
    stats->total_bases += len;

    if (stats->min_length == -1 || len < stats->min_length) {
        stats->min_length = len;
    }

    if (len > stats->max_length) {
        stats->max_length = len;
    }

    if (ensure_per_base_capacity(stats, len) != 0) {
        return 1;
    }

    if (ensure_length_hist_capacity(stats, len) != 0) {
        return 1;
    }

    stats->length_hist[len]++;

    for (long i = 0; i < len; i++) {
        char base = (char) toupper((unsigned char) record->sequence[i]);
        int q = (int) record->quality[i] - 33;

        if (base == 'G' || base == 'C') {
            stats->gc_bases++;
            stats->per_base_gc_counts[i]++;
        }

        if (base == 'N') {
            stats->n_bases++;
            stats->per_base_n_counts[i]++;
        }

        stats->total_quality += q;
        stats->per_base_quality_sum[i] += q;
        stats->per_base_counts[i]++;
    }

    return 0;
}

double qc_stats_mean_length(const QcStats *stats) {
    if (stats->total_reads == 0) {
        return 0.0;
    }

    return (double) stats->total_bases / (double) stats->total_reads;
}

double qc_stats_gc_percent(const QcStats *stats) {
    if (stats->total_bases == 0) {
        return 0.0;
    }

    return ((double) stats->gc_bases / (double) stats->total_bases) * 100.0;
}

double qc_stats_n_percent(const QcStats *stats) {
    if (stats->total_bases == 0) {
        return 0.0;
    }

    return ((double) stats->n_bases / (double) stats->total_bases) * 100.0;
}

double qc_stats_mean_quality(const QcStats *stats) {
    if (stats->total_bases == 0) {
        return 0.0;
    }

    return (double) stats->total_quality / (double) stats->total_bases;
}

double qc_stats_mean_quality_at_cycle(const QcStats *stats, long cycle) {
    if (cycle < 1 || cycle > stats->per_base_capacity) {
        return 0.0;
    }

    long idx = cycle - 1;
    if (stats->per_base_counts[idx] == 0) {
        return 0.0;
    }

    return (double) stats->per_base_quality_sum[idx] / (double) stats->per_base_counts[idx];
}

double qc_stats_gc_percent_at_cycle(const QcStats *stats, long cycle) {
    if (cycle < 1 || cycle > stats->per_base_capacity) {
        return 0.0;
    }

    long idx = cycle - 1;
    if (stats->per_base_counts[idx] == 0) {
        return 0.0;
    }

    return ((double) stats->per_base_gc_counts[idx] / (double) stats->per_base_counts[idx]) * 100.0;
}

double qc_stats_n_percent_at_cycle(const QcStats *stats, long cycle) {
    if (cycle < 1 || cycle > stats->per_base_capacity) {
        return 0.0;
    }

    long idx = cycle - 1;
    if (stats->per_base_counts[idx] == 0) {
        return 0.0;
    }

    return ((double) stats->per_base_n_counts[idx] / (double) stats->per_base_counts[idx]) * 100.0;
}
