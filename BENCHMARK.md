# Chatbot Build & Runtime Benchmark Results

Benchmark comparing C (GCC and Clang, C17 and C23 standards) and Zig implementations.

**System:** macOS arm64  
**Date:** 2026-01-12

## Compiler Versions (Local)


| Compiler    | Version                                      |
| ----------- | -------------------------------------------- |
| GCC (macOS) | Apple Clang 17.0.0 (gcc is aliased to clang) |
| Clang       | Homebrew Clang 21.1.8                        |
| Zig         | 0.15.2                                       |


## Runtime Results


| Version   | Execution Time | Relative        |
| --------- | -------------- | --------------- |
| GCC C17   | 228ms          | 2.23x           |
| GCC C23   | 102ms          | 1.00x (fastest) |
| Clang C17 | 107ms          | 1.04x           |
| Clang C23 | 106ms          | 1.03x           |
| Zig       | 628ms          | 6.15x           |


## Binary Sizes


| Version   | Size |
| --------- | ---- |
| GCC C17   | 33K  |
| GCC C23   | 33K  |
| Clang C17 | 33K  |
| Clang C23 | 33K  |
| Zig       | 1.3M |


## Analysis

### C Compilers

- **GCC C23** was fastest in this run (times vary between runs)
- **Clang** produces consistently fast binaries across C17/C23
- On macOS, `gcc` is actually Apple Clang; real GCC is tested on CI (Ubuntu)

### Zig

- Slower due to:
  - GeneralPurposeAllocator overhead vs C's stack allocation
  - Zig 0.15 buffered I/O system overhead
  - Additional runtime safety checks
- Much larger binary (embeds stdlib, no libc dependency)

### Binary Size

- All C versions: 33K (links against system libc)
  - Zig: 1.3M (self-contained, no external dependencies)

## Build Configuration


| Compiler  | Flags                                    |
| --------- | ---------------------------------------- |
| GCC C17   | `gcc -std=c17 -Wall -Wextra -pedantic`   |
| GCC C23   | `gcc -std=c23 -Wall -Wextra -pedantic`   |
| Clang C17 | `clang -std=c17 -Wall -Wextra -pedantic` |
| Clang C23 | `clang -std=c23 -Wall -Wextra -pedantic` |
| Zig       | `zig build` (debug mode)                 |


## Notes

- All versions produce identical output
- C23 falls back to `-std=c2x` on older compilers
- Times vary between runs; relative performance is more meaningful
- CI tests both GCC and Clang on Ubuntu

## Running the Benchmarks

```bash
# Full runtime benchmark (all compilers)
./run_benchmarks.sh

# Build-only benchmark
./benchmark.sh

# Show detected compilers
cd c && make info
```

