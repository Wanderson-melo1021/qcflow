# QCFLOW

**QCFLOW** is a high-performance FASTQ quality control tool written in C.

It is designed to process large sequencing datasets efficiently using streaming and gzip-aware parsing, while remaining modular and pipeline-friendly.

## 🧠 Design Goals

* High performance (C-based core)
* Streaming FASTQ/FASTQ.GZ support
* Minimal dependencies
* Modular integration with pipelines
* Reproducible output format

## ⚙️ Features (v0.1-alpha)

* FASTQ and FASTQ.GZ parsing
* Single-end and paired-end support
* Per-base quality metrics
* GC content calculation
* N content calculation
* Read length distribution
* Summary statistics
* TSV output for all metrics
* Optional report generation via external script

## 🚀 Usage

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

## 📂 Output

* `<prefix>.summary.tsv`
* `<prefix>.per_base_quality.tsv`
* `<prefix>.per_base_gc.tsv`
* `<prefix>.per_base_n.tsv`
* `<prefix>.length_distribution.tsv`
* optional:

  * HTML report
  * PNG plots

## 🧩 Integration

QCFLOW is designed to be used as:

* standalone QC tool
* module inside larger pipelines (e.g., RNAFLOW)

## ⚡ Performance

Handles millions of reads efficiently using:

* streaming parsing
* direct gzip handling (zlib)
* minimal memory overhead

## 📌 Status

Current version: **v0.1-alpha**

## 🔮 Future Features

* adapter detection
* duplication metrics
* overrepresented sequences
* quality histograms
* batch mode

