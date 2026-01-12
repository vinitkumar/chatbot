# Chatbot

A simple keyword-based chatbot with implementations in both C and Zig.

## Benchmark

See [BENCHMARK.md](BENCHMARK.md) for build time and executable size comparisons.

Quick summary:
- **C**: 93ms build time, 33K executable
- **Zig**: 710ms build time, 1.3M executable

To run the benchmark:
```bash
./benchmark.sh
```

## Building

### C Version

```bash
cd c
make
./chat
```

See `c/Makefile` for more details.

### Zig Version

```bash
cd zig
zig build-exe src/main.zig -femit-bin=zig-out/bin/chat
./zig-out/bin/chat
```

Or use the Zig build system:

```bash
cd zig
zig build run
```

See `zig/README.md` for more details.

## Testing

### C Version

No automated tests (original implementation).

### Zig Version

```bash
cd zig
zig test src/chatbot.zig
```

Or:

```bash
cd zig
zig build test
```
