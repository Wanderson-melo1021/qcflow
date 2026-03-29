# QCFLOW Documentation

## 1. Overview

QCFLOW is a FASTQ quality control tool written in C, designed for efficient processing of large sequencing datasets using streaming and direct gzip handling.

It supports both single-end (SE) and paired-end (PE) data and is intended to be used either as a standalone tool or as a module within pipelines such as RNAFLOW.

---

## 2. Architecture

QCFLOW is composed of modular components:

* **CLI layer (cli.c)**

  * argument parsing
  * configuration setup

* **I/O layer (gzip_io.c)**

  * handles FASTQ and FASTQ.GZ transparently
  * uses zlib for compressed input

* **Reader (fastq_reader.c)**

  * parses FASTQ records
  * validates structure

* **QC statistics (qc_stats.c)**

  * read count
  * base count
  * GC content
  * N content
  * quality metrics
  * length distribution

* **Writer (writer.c)**

  * outputs structured TSV files
  * ensures consistent format

---

## 3. Input

### Supported formats

* `.fastq`
* `.fastq.gz`

### Modes

* Single-end:

  * `--in`

* Paired-end:

  * `--in1` + `--in2`

---

## 4. Output

For each run, QCFLOW generates:

### Summary

* `<prefix>.summary.tsv`

### Per-base metrics

* `<prefix>.per_base_quality.tsv`
* `<prefix>.per_base_gc.tsv`
* `<prefix>.per_base_n.tsv`

### Distribution

* `<prefix>.length_distribution.tsv`

### Optional (report)

* `<prefix>_prereport.html`
* PNG plots

---

## 5. CLI Interface

### Single-end

```bash
qcflow qc \
  --in sample.fastq.gz \
  --out results/qc/sample
```

### Paired-end

```bash
qcflow qc \
  --in1 sample_R1.fastq.gz \
  --in2 sample_R2.fastq.gz \
  --out results/qc/sample
```

### With report

```bash
qcflow qc \
  --in1 sample_R1.fastq.gz \
  --in2 sample_R2.fastq.gz \
  --out results/qc/sample \
  --report \
  --report-script path/to/qc_prereport.R
```

⚠️ `--report` requires `--report-script`

---

## 6. Metrics Computed

* total reads
* total bases
* min / max / mean read length
* GC content (global and per-base)
* N content (global and per-base)
* mean quality score
* per-base quality profile
* read length distribution

---

## 7. Paired-End Behavior

When running in paired-end mode:

* R1 and R2 are processed independently
* outputs are unified in a single file
* `scope` column differentiates:

  * `pair`
  * `R1`
  * `R2`

Additional checks:

* read count consistency between mates

---

## 8. Error Handling

QCFLOW enforces strict validation:

* malformed FASTQ → error
* truncated records → error
* missing files → error
* gzip corruption → error

Execution stops immediately on failure.

---

## 9. Integration

QCFLOW is designed to be used externally by pipelines.

Example:

```bash
QCFLOW_BIN=/path/to/qcflow
QCFLOW_R_SCRIPT=/path/to/script.R
```

RNAFLOW integration:

* QCFLOW is called per sample
* report generation is externally controlled

---

## 10. Performance Considerations

* streaming-based parsing
* gzip handled without full decompression
* low memory footprint
* scalable to millions of reads

---

## 11. Current Status

Version: **v0.1-alpha**

Functional components:

* FASTQ parsing ✔️
* QC metrics ✔️
* PE support ✔️
* TSV output ✔️
* report integration ✔️

---

## 12. Future Work

* adapter detection
* duplication metrics
* overrepresented sequences
* batch mode execution
* extended QC metrics

