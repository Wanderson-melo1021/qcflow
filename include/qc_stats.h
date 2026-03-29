#ifndef QC_STATS_H
#define QC_STATS_H

#include "fastq_reader.h"

typedef struct {
    long total_reads;
    long total_bases;
    long min_length;
    long max_length;
    long gc_bases;
    long n_bases;

    long total_quality;
    long *per_base_quality_sum;
    long *per_base_counts;
    long *per_base_gc_counts;
    long *per_base_n_counts;
    long per_base_capacity;

    long *length_hist;
    long length_hist_capacity;
} QcStats;

void qc_stats_init(QcStats *stats);
void qc_stats_free(QcStats *stats);
int qc_stats_update(QcStats *stats, const FastqRecord *record);

double qc_stats_mean_length(const QcStats *stats);
double qc_stats_gc_percent(const QcStats *stats);
double qc_stats_n_percent(const QcStats *stats);
double qc_stats_mean_quality(const QcStats *stats);
double qc_stats_mean_quality_at_cycle(const QcStats *stats, long cycle);
double qc_stats_gc_percent_at_cycle(const QcStats *stats, long cycle);
double qc_stats_n_percent_at_cycle(const QcStats *stats, long cycle);

#endif
