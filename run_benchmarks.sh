#!/bin/bash

set -e

echo "=========================================="
echo "Chatbot Runtime Benchmark & Testing"
echo "=========================================="
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

# Test C17 version
echo "=========================================="
echo "C17 Version Testing & Benchmarking"
echo "=========================================="
echo ""
echo "Running with test inputs..."

C17_START=$(date +%s%N)
c/./chat-c17 < test_inputs.txt > /tmp/c17_output.txt 2>&1
C17_END=$(date +%s%N)
C17_TIME=$(( (C17_END - C17_START) / 1000000 ))  # milliseconds

echo "Output:"
cat /tmp/c17_output.txt
echo ""
echo "Execution time: ${C17_TIME}ms"
echo ""

# Test C23 version
echo "=========================================="
echo "C23 Version Testing & Benchmarking"
echo "=========================================="
echo ""
echo "Running with test inputs..."

C23_START=$(date +%s%N)
c/./chat-c23 < test_inputs.txt > /tmp/c23_output.txt 2>&1
C23_END=$(date +%s%N)
C23_TIME=$(( (C23_END - C23_START) / 1000000 ))  # milliseconds

echo "Output:"
cat /tmp/c23_output.txt
echo ""
echo "Execution time: ${C23_TIME}ms"
echo ""

# Test Zig version
echo "=========================================="
echo "Zig Version Testing & Benchmarking"
echo "=========================================="
echo ""
echo "Running with test inputs..."

ZIG_START=$(date +%s%N)
zig/zig-out/bin/./chat < test_inputs.txt > /tmp/zig_output.txt 2>&1
ZIG_END=$(date +%s%N)
ZIG_TIME=$(( (ZIG_END - ZIG_START) / 1000000 ))  # milliseconds

echo "Output:"
cat /tmp/zig_output.txt
echo ""
echo "Execution time: ${ZIG_TIME}ms"
echo ""

# Compare outputs
echo "=========================================="
echo "Output Comparison"
echo "=========================================="
echo ""

OUTPUTS_MATCH=true
if diff -q /tmp/c17_output.txt /tmp/c23_output.txt > /dev/null 2>&1; then
  echo "✓ C17 and C23 outputs are identical"
else
  echo "✗ C17 and C23 outputs differ"
  OUTPUTS_MATCH=false
fi

if diff -q /tmp/c17_output.txt /tmp/zig_output.txt > /dev/null 2>&1; then
  echo "✓ C17 and Zig outputs are identical"
else
  echo "✗ C17 and Zig outputs differ"
  OUTPUTS_MATCH=false
fi

echo ""

# Runtime comparison
echo "=========================================="
echo "Runtime Comparison"
echo "=========================================="
echo ""

# Find the fastest
MIN_TIME=$C17_TIME
FASTEST="C17"
if [ $C23_TIME -lt $MIN_TIME ]; then
  MIN_TIME=$C23_TIME
  FASTEST="C23"
fi
if [ $ZIG_TIME -lt $MIN_TIME ]; then
  MIN_TIME=$ZIG_TIME
  FASTEST="Zig"
fi

echo "Fastest: $FASTEST (${MIN_TIME}ms)"
echo ""
echo "Summary:"
echo "  C17: ${C17_TIME}ms"
echo "  C23: ${C23_TIME}ms"
echo "  Zig: ${ZIG_TIME}ms"
echo ""

# Show ratios relative to fastest
if [ $MIN_TIME -gt 0 ]; then
  C17_RATIO=$(echo "scale=2; $C17_TIME / $MIN_TIME" | bc)
  C23_RATIO=$(echo "scale=2; $C23_TIME / $MIN_TIME" | bc)
  ZIG_RATIO=$(echo "scale=2; $ZIG_TIME / $MIN_TIME" | bc)
  echo "Relative to fastest:"
  echo "  C17: ${C17_RATIO}x"
  echo "  C23: ${C23_RATIO}x"
  echo "  Zig: ${ZIG_RATIO}x"
  echo ""
fi

# Binary sizes
echo "=========================================="
echo "Binary Sizes"
echo "=========================================="
echo ""
C17_SIZE=$(ls -lh c/chat-c17 | awk '{print $5}')
C23_SIZE=$(ls -lh c/chat-c23 | awk '{print $5}')
ZIG_SIZE=$(ls -lh zig/zig-out/bin/chat | awk '{print $5}')
echo "  C17: $C17_SIZE"
echo "  C23: $C23_SIZE"
echo "  Zig: $ZIG_SIZE"
echo ""
