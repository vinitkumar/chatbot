# Chatbot Build Benchmark Results

Benchmark comparing C and Zig implementations of the chatbot.

**System:** macOS arm64  
**Date:** 2026-01-12

## Results

| Metric | C | Zig | Difference |
|--------|---|-----|-----------|
| Build Time | 93ms | 710ms | Zig is 7.63x slower |
| Executable Size | 33K | 1.3M | Zig is 39x larger |

## Analysis

### Build Time
- **C (93ms)**: Fast compilation using GCC with minimal optimization
- **Zig (710ms)**: Longer compilation time due to Zig's more comprehensive compiler

The C version compiles significantly faster due to:
- Simpler compilation pipeline
- No build system overhead (direct gcc command)
- Minimal type checking and analysis

### Executable Size
- **C (33K)**: Small, minimal runtime
- **Zig (1.3M)**: Larger due to Zig's standard library and runtime

The Zig executable is larger because:
- Zig stdlib is embedded in the binary
- More comprehensive runtime features
- GeneralPurposeAllocator adds overhead

## Notes

- Both versions are unoptimized builds
- C build uses `-std=c11 -Wall -Wextra -pedantic`
- Zig build uses default optimization level
- Times are from cold builds (no cache)

## Running the Benchmark

```bash
./benchmark.sh
```

This will clean, rebuild both versions, and compare build times and sizes.
