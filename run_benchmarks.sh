#!/bin/bash

set -e

echo "=========================================="
echo "Chatbot Runtime Benchmark & Testing"
echo "=========================================="
echo ""

# Build both versions
echo "Building both versions..."
echo ""

cd c
make clean > /dev/null 2>&1
make > /dev/null 2>&1
cd ..

cd zig
rm -rf zig-out .zig-cache > /dev/null 2>&1
mkdir -p zig-out/bin
zig build-exe src/main.zig -femit-bin=zig-out/bin/chat > /dev/null 2>&1
cd ..

echo "Build complete!"
echo ""

# Test C version
echo "=========================================="
echo "C Version Testing & Benchmarking"
echo "=========================================="
echo ""
echo "Running with test inputs..."

C_START=$(date +%s%N)
c/./chat < test_inputs.txt > /tmp/c_output.txt 2>&1
C_END=$(date +%s%N)
C_TIME=$(( (C_END - C_START) / 1000000 ))  # milliseconds

echo "Output:"
cat /tmp/c_output.txt
echo ""
echo "Execution time: ${C_TIME}ms"
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

if diff -q /tmp/c_output.txt /tmp/zig_output.txt > /dev/null 2>&1; then
  echo "✓ Outputs are identical"
else
  echo "✗ Outputs differ"
  echo ""
  echo "C output:"
  cat /tmp/c_output.txt
  echo ""
  echo "Zig output:"
  cat /tmp/zig_output.txt
fi

echo ""

# Runtime comparison
echo "=========================================="
echo "Runtime Comparison"
echo "=========================================="
echo ""

if [ $C_TIME -lt $ZIG_TIME ]; then
  RATIO=$(echo "scale=2; $ZIG_TIME / $C_TIME" | bc)
  echo "C version is ${RATIO}x faster"
else
  RATIO=$(echo "scale=2; $C_TIME / $ZIG_TIME" | bc)
  echo "Zig version is ${RATIO}x faster"
fi

echo ""
echo "Summary:"
echo "  C:   ${C_TIME}ms"
echo "  Zig: ${ZIG_TIME}ms"
echo ""
