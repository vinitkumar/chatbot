# Chatbot

A simple keyword-based chatbot with implementations in both C and Zig.

## Benchmarks

### Build Time & Size

See [BENCHMARK.md](BENCHMARK.md) for detailed build metrics.

| Metric | C | Zig | Difference |
|--------|---|-----|-----------|
| Build Time | 93ms | 710ms | Zig is 7.63x slower |
| Executable Size | 33K | 1.3M | Zig is 39x larger |

Run build benchmark:
```bash
./benchmark.sh
```

### Runtime Performance

Both versions produce identical output. Runtime measurements with test inputs:

| Metric | C | Zig | Difference |
|--------|---|-----|-----------|
| Execution Time | 286ms | 526ms | C is 1.83x faster |

Run runtime benchmark:
```bash
./run_benchmarks.sh
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
