CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -std=c11 -Iinclude
LDFLAGS = -lz

SRC = src/c/main.c src/c/cli.c src/c/gzip_io.c src/c/fastq_reader.c src/c/qc_stats.c src/c/writer.c
OUT = bin/qcflow

all: $(OUT)

$(OUT): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -rf bin

run: $(OUT)
	./$(OUT) qc --in tests/data/tiny_se.fastq.gz --out results/qc/tiny
