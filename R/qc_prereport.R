args <- commandArgs(trailingOnly = TRUE)

if (length(args) != 1) {
  stop("Usage: Rscript R/qc_prereport.R <output_prefix>")
}

prefix <- args[1]

summary_file <- paste0(prefix, ".summary.tsv")
quality_file <- paste0(prefix, ".per_base_quality.tsv")
gc_file <- paste0(prefix, ".per_base_gc.tsv")
n_file <- paste0(prefix, ".per_base_n.tsv")
length_file <- paste0(prefix, ".length_distribution.tsv")
html_file <- paste0(prefix, "_prereport.html")

suppressPackageStartupMessages({
  library(ggplot2)
  library(readr)
  library(dplyr)
  library(rmarkdown)
})

summary_df <- read_tsv(summary_file, show_col_types = FALSE)
quality_df <- read_tsv(quality_file, show_col_types = FALSE)
gc_df <- read_tsv(gc_file, show_col_types = FALSE)
n_df <- read_tsv(n_file, show_col_types = FALSE)
length_df <- read_tsv(length_file, show_col_types = FALSE)

plot_quality <- ggplot(quality_df, aes(x = cycle, y = mean_quality, group = mate, color = mate)) +
  geom_line(linewidth = 1) +
  geom_point() +
  labs(
    title = "Mean quality per cycle",
    x = "Cycle",
    y = "Mean quality"
  ) +
  theme_minimal()

plot_gc <- ggplot(gc_df, aes(x = cycle, y = gc_percent, group = mate, color = mate)) +
  geom_line(linewidth = 1) +
  geom_point() +
  labs(
    title = "GC percentage per cycle",
    x = "Cycle",
    y = "GC %"
  ) +
  theme_minimal()

plot_n <- ggplot(n_df, aes(x = cycle, y = n_percent, group = mate, color = mate)) +
  geom_line(linewidth = 1) +
  geom_point() +
  labs(
    title = "N percentage per cycle",
    x = "Cycle",
    y = "N %"
  ) +
  theme_minimal()

plot_length <- ggplot(length_df, aes(x = length, y = count, fill = mate)) +
  geom_col(position = "dodge") +
  labs(
    title = "Length distribution",
    x = "Read length",
    y = "Count"
  ) +
  theme_minimal()

png_quality <- paste0(prefix, "_quality.png")
png_gc <- paste0(prefix, "_gc.png")
png_n <- paste0(prefix, "_n.png")
png_length <- paste0(prefix, "_length.png")

ggsave(png_quality, plot_quality, width = 7, height = 5, dpi = 150)
ggsave(png_gc, plot_gc, width = 7, height = 5, dpi = 150)
ggsave(png_n, plot_n, width = 7, height = 5, dpi = 150)
ggsave(png_length, plot_length, width = 7, height = 5, dpi = 150)

html_lines <- c(
  "<html>",
  "<head><meta charset='UTF-8'><title>qcflow pre-report</title></head>",
  "<body>",
  paste0("<h1>qcflow pre-report: ", basename(prefix), "</h1>"),
  "<h2>Summary</h2>",
  "<table border='1' cellspacing='0' cellpadding='6'>",
  "<tr><th>metric</th><th>scope</th><th>value</th></tr>"
)

for (i in seq_len(nrow(summary_df))) {
  html_lines <- c(
    html_lines,
    paste0(
      "<tr><td>", summary_df$metric[i],
      "</td><td>", summary_df$scope[i],
      "</td><td>", summary_df$value[i],
      "</td></tr>"
    )
  )
}

html_lines <- c(
  html_lines,
  "</table>",
  "<h2>Mean quality per cycle</h2>",
  paste0("<img src='", basename(png_quality), "' width='700'>"),
  "<h2>GC percentage per cycle</h2>",
  paste0("<img src='", basename(png_gc), "' width='700'>"),
  "<h2>N percentage per cycle</h2>",
  paste0("<img src='", basename(png_n), "' width='700'>"),
  "<h2>Length distribution</h2>",
  paste0("<img src='", basename(png_length), "' width='700'>"),
  "</body>",
  "</html>"
)

writeLines(html_lines, html_file)

cat("Pre-report written to:", html_file, "\n")
