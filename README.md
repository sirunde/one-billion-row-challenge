# One Billion Row Challenge in C++

A high-performance C++ implementation of the [One Billion Row Challenge (1BRC)](https://github.com/gunnarmorling/1brc), focused on efficient file reading, parallel processing, and fast temperature aggregation.

## Overview

The One Billion Row Challenge processes a text file containing one billion temperature measurements from weather stations. Each line contains a station name and its recorded temperature, separated by a semicolon.

```text
Hamburg;12.0
Bulawayo;8.9
Palembang;38.8
Istanbul;6.2
```

The goal is to calculate the minimum, mean, and maximum temperature for each station and output the results in alphabetical order.

This project explores different file reading strategies and data structures to improve performance when processing large datasets.

## Features

- **Parallel processing:** Uses multiple threads to process large input files concurrently.
- **Memory-mapped file I/O:** Uses `mmap` to access file contents efficiently.
- **Alternative file reading:** Includes a `pread`-based implementation for comparison.
- **Efficient temperature parsing:** Converts temperature values directly into integer representations to reduce conversion overhead.
- **Fast aggregation:** Uses `parallel_flat_hash_map` to improve station lookup and aggregation performance.
- **Configurable thread count:** Allows the number of worker threads to be specified at runtime.

## Implementations

| File | Description |
|---|---|
| `main.cpp` | Memory-mapped file implementation using `mmap`. |
| `pmain.cpp` | File reading implementation using `pread`. |
| `Makefile` | Build and run commands. |
| `parallel_hashmap/` | Parallel hash map dependency. |
| `data/` | Input data and related files. |
| `src/` | Additional source files. |

## Requirements

- A C++ compiler with C++11 or later support
- GNU Make
- POSIX-compatible operating system with `mmap`, `pread`, and pthread support
- Sufficient memory and disk space for the input dataset

The project was developed and tested with `g++` on a Unix-like environment.

## Build

Clone the repository and enter the project directory:

```bash
git clone https://github.com/sirunde/one-billion-row-challenge.git
cd one-billion-row-challenge
```

Compile the `mmap` implementation:

```bash
g++ -pthread -g -Wall -Wextra -O3 mmain.cpp -o main
```

The command above follows the compiler flags used in the project. Make sure the source file and its dependencies are available in the repository.

## Run

Run the program with an input file and an optional thread count:

```bash
./main measurements.txt 8
```

Arguments:

- `measurements.txt`: Path to the input file.
- `8`: Number of worker threads. The default is 4 when the thread count is omitted.

The program processes the input and calculates the temperature statistics for each weather station.

To compare the alternative implementation, compile and run `pmain.cpp` using the same compiler settings and the appropriate output configuration.

## Benchmark Results

The following are the best results recorded during development:

| Dataset | Threads | Best recorded time |
|---|---:|---:|
| 1 billion rows | 8 | 20.21 seconds |
| 100 million rows | 8 | 1.5 seconds |

These are development measurements, not independently verified benchmark results. Performance depends on the CPU, storage device, memory bandwidth, compiler, and operating system.

For meaningful comparisons, run each implementation on the same machine with the same dataset and thread count.

## Optimization Experiments

Several approaches were explored during development:

1. **`pread` vs. `mmap`:** Compared positional file reads with memory-mapped access. The results showed no consistent advantage for `pread`, so `mmap` was retained as the preferred approach.
2. **Custom parsing:** Replaced standard string-to-number conversion with a custom temperature parser to reduce conversion overhead.
3. **Integer temperature representation:** Stores temperatures in tenths of a degree to avoid unnecessary floating-point conversion during aggregation.
4. **Hash map optimization:** Replaced the standard unordered map with `parallel_flat_hash_map` after observing improved runtime.
5. **Custom hashing:** Experimented with a custom hash implementation, but it did not provide a consistent performance improvement.
6. **SIMD:** Investigated SIMD optimization as a possible future improvement.

## Future Improvements

- Explore SIMD-based parsing and aggregation.
- Improve thread scheduling and workload distribution.
- Benchmark different file chunk sizes and thread counts.
- Compare memory usage and throughput across file reading strategies.
- Add automated correctness tests against a reference implementation.

## Acknowledgments

- [The One Billion Row Challenge](https://github.com/gunnarmorling/1brc) by Gunnar Morling.
- [parallel-hashmap](https://github.com/greg7mdp/parallel-hashmap), used for efficient hash-based aggregation.

## License

Refer to the licenses of the project dependencies and the original One Billion Row Challenge before redistributing code or datasets.
