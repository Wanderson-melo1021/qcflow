#ifndef WRITER_H
#define WRITER_H

#include "qc_stats.h"

int write_summary_tsv_se(const char *output_prefix, const QcStats *stats);
int write_per_base_quality_tsv_se(const char *output_prefix, const QcStats *stats);
int write_per_base_gc_tsv_se(const char *output_prefix, const QcStats *stats);
int write_per_base_n_tsv_se(const char *output_prefix, const QcStats *stats);
int write_length_distribution_tsv_se(const char *output_prefix, const QcStats *stats);

int write_summary_tsv_pe(const char *output_prefix,
                         const QcStats *stats1,
                         const QcStats *stats2);

int write_per_base_quality_tsv_pe(const char *output_prefix,
                                  const QcStats *stats1,
                                  const QcStats *stats2);

int write_per_base_gc_tsv_pe(const char *output_prefix,
                             const QcStats *stats1,
                             const QcStats *stats2);

int write_per_base_n_tsv_pe(const char *output_prefix,
                            const QcStats *stats1,
                            const QcStats *stats2);

int write_length_distribution_tsv_pe(const char *output_prefix,
                                     const QcStats *stats1,
                                     const QcStats *stats2);

#endif
