#!/bin/bash

set -e

echo "=========================================="
echo "Chatbot Runtime Benchmark & Testing"
echo "=========================================="
echo ""

# Show compiler versions
echo "Compiler Versions:"
echo "  GCC:   $(gcc --version | head -1)"
echo "  Clang: $(clang --version | head -1)"
echo "  Zig:   $(zig version)"
echo ""

# Build all versions
echo "Building all versions..."
echo ""

cd c
make clean > /dev/null 2>&1
make > /dev/null 2>&1
cd ..

cd zig
rm -rf zig-out .zig-cache > /dev/null 2>&1
mkdir -p zig-out/bin
zig build > /dev/null 2>&1
cd ..

echo "Build complete!"
echo ""

# Run benchmarks
echo "=========================================="
echo "Running Benchmarks..."
echo "=========================================="
echo ""

# GCC C17
echo -n "GCC C17:   "
GCC_C17_START=$(date +%s%N)
c/chat-gcc-c17 < test_inputs.txt > /tmp/gcc_c17_output.txt 2>&1
GCC_C17_END=$(date +%s%N)
GCC_C17_TIME=$(( (GCC_C17_END - GCC_C17_START) / 1000000 ))
echo "${GCC_C17_TIME}ms"

# GCC C23
echo -n "GCC C23:   "
GCC_C23_START=$(date +%s%N)
c/chat-gcc-c23 < test_inputs.txt > /tmp/gcc_c23_output.txt 2>&1
GCC_C23_END=$(date +%s%N)
GCC_C23_TIME=$(( (GCC_C23_END - GCC_C23_START) / 1000000 ))
echo "${GCC_C23_TIME}ms"

# Clang C17
echo -n "Clang C17: "
CLANG_C17_START=$(date +%s%N)
c/chat-clang-c17 < test_inputs.txt > /tmp/clang_c17_output.txt 2>&1
CLANG_C17_END=$(date +%s%N)
CLANG_C17_TIME=$(( (CLANG_C17_END - CLANG_C17_START) / 1000000 ))
echo "${CLANG_C17_TIME}ms"

# Clang C23
echo -n "Clang C23: "
CLANG_C23_START=$(date +%s%N)
c/chat-clang-c23 < test_inputs.txt > /tmp/clang_c23_output.txt 2>&1
CLANG_C23_END=$(date +%s%N)
CLANG_C23_TIME=$(( (CLANG_C23_END - CLANG_C23_START) / 1000000 ))
echo "${CLANG_C23_TIME}ms"

# Zig
echo -n "Zig:       "
ZIG_START=$(date +%s%N)
zig/zig-out/bin/chat < test_inputs.txt > /tmp/zig_output.txt 2>&1
ZIG_END=$(date +%s%N)
ZIG_TIME=$(( (ZIG_END - ZIG_START) / 1000000 ))
echo "${ZIG_TIME}ms"

echo ""

# Compare outputs
echo "=========================================="
echo "Output Comparison"
echo "=========================================="
echo ""

REFERENCE="/tmp/gcc_c17_output.txt"

for file in /tmp/gcc_c23_output.txt /tmp/clang_c17_output.txt /tmp/clang_c23_output.txt /tmp/zig_output.txt; do
    name=$(basename "$file" _output.txt | tr '_' ' ')
    if diff -q "$REFERENCE" "$file" > /dev/null 2>&1; then
        echo "✓ $name matches GCC C17"
    else
        echo "✗ $name differs from GCC C17"
    fi
done

echo ""

# Find fastest
echo "=========================================="
echo "Runtime Comparison"
echo "=========================================="
echo ""

MIN_TIME=$GCC_C17_TIME
FASTEST="GCC C17"

if [ $GCC_C23_TIME -lt $MIN_TIME ]; then MIN_TIME=$GCC_C23_TIME; FASTEST="GCC C23"; fi
if [ $CLANG_C17_TIME -lt $MIN_TIME ]; then MIN_TIME=$CLANG_C17_TIME; FASTEST="Clang C17"; fi
if [ $CLANG_C23_TIME -lt $MIN_TIME ]; then MIN_TIME=$CLANG_C23_TIME; FASTEST="Clang C23"; fi
if [ $ZIG_TIME -lt $MIN_TIME ]; then MIN_TIME=$ZIG_TIME; FASTEST="Zig"; fi

echo "Fastest: $FASTEST (${MIN_TIME}ms)"
echo ""

# Calculate ratios
if [ $MIN_TIME -gt 0 ]; then
    GCC_C17_RATIO=$(echo "scale=2; $GCC_C17_TIME / $MIN_TIME" | bc)
    GCC_C23_RATIO=$(echo "scale=2; $GCC_C23_TIME / $MIN_TIME" | bc)
    CLANG_C17_RATIO=$(echo "scale=2; $CLANG_C17_TIME / $MIN_TIME" | bc)
    CLANG_C23_RATIO=$(echo "scale=2; $CLANG_C23_TIME / $MIN_TIME" | bc)
    ZIG_RATIO=$(echo "scale=2; $ZIG_TIME / $MIN_TIME" | bc)

    echo "Results (time / relative):"
    printf "  %-12s %6sms  %5sx\n" "GCC C17:" "$GCC_C17_TIME" "$GCC_C17_RATIO"
    printf "  %-12s %6sms  %5sx\n" "GCC C23:" "$GCC_C23_TIME" "$GCC_C23_RATIO"
    printf "  %-12s %6sms  %5sx\n" "Clang C17:" "$CLANG_C17_TIME" "$CLANG_C17_RATIO"
    printf "  %-12s %6sms  %5sx\n" "Clang C23:" "$CLANG_C23_TIME" "$CLANG_C23_RATIO"
    printf "  %-12s %6sms  %5sx\n" "Zig:" "$ZIG_TIME" "$ZIG_RATIO"
    echo ""
fi

# Binary sizes
echo "=========================================="
echo "Binary Sizes"
echo "=========================================="
echo ""
printf "  %-12s %s\n" "GCC C17:" "$(ls -lh c/chat-gcc-c17 | awk '{print $5}')"
printf "  %-12s %s\n" "GCC C23:" "$(ls -lh c/chat-gcc-c23 | awk '{print $5}')"
printf "  %-12s %s\n" "Clang C17:" "$(ls -lh c/chat-clang-c17 | awk '{print $5}')"
printf "  %-12s %s\n" "Clang C23:" "$(ls -lh c/chat-clang-c23 | awk '{print $5}')"
printf "  %-12s %s\n" "Zig:" "$(ls -lh zig/zig-out/bin/chat | awk '{print $5}')"
echo ""
