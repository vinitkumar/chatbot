# Chatbot Build & Runtime Benchmark Results

Benchmark comparing C (C17 and C23 standards) and Zig implementations of the chatbot.

**System:** macOS arm64  
**Date:** 2026-01-12

## Build Results

| Metric | C17 | C23 | Zig | Notes |
|--------|-----|-----|-----|-------|
| Build Time | ~90ms | ~90ms | ~710ms | Zig ~8x slower |
| Executable Size | 33K | 33K | 1.3M | Zig ~40x larger |

## Runtime Results

| Version | Execution Time | Relative |
|---------|---------------|----------|
| C17 | 147ms | 1.00x |
| C23 | 197ms | 1.34x |
| Zig | 459ms | 3.12x |

## Analysis

### Build Time
- **C (C17/C23)**: Fast compilation using GCC with minimal optimization
- **Zig**: Longer compilation time due to more comprehensive compiler

### Executable Size
- **C (33K)**: Small, minimal runtime, links against system libc
- **Zig (1.3M)**: Larger due to embedded standard library and runtime

### Runtime Performance
- **C17** is the fastest, likely due to mature compiler optimizations
- **C23** is slightly slower (may vary based on compiler version)
- **Zig** is slower due to:
  - GeneralPurposeAllocator overhead vs C's stack allocation
  - New Zig 0.15 buffered I/O system overhead
  - Additional safety checks

## Build Configuration

- **C17**: `gcc -std=c17 -Wall -Wextra -pedantic`
- **C23**: `gcc -std=c23 -Wall -Wextra -pedantic` (falls back to `-std=c2x` on older compilers)
- **Zig**: `zig build` (debug mode, Zig 0.15.2)

## Notes

- All versions produce identical output
- Times are from cold builds (no cache)
- Runtime measured with same test input file
- Zig uses pure Zig I/O (no libc linking)

## Running the Benchmarks

```bash
# Full runtime benchmark
./run_benchmarks.sh

# Build-only benchmark
./benchmark.sh
```
