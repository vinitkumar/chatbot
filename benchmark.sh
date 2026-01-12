#!/bin/bash

set -e

echo "=========================================="
echo "Chatbot Build Benchmark"
echo "=========================================="
echo ""

# C Version Benchmark
echo "C Version Benchmark"
echo "------------------------------------------"

cd c
echo "Cleaning..."
make clean > /dev/null 2>&1

echo "Building C version..."
C_START=$(date +%s%N)
make > /dev/null 2>&1
C_END=$(date +%s%N)
C_TIME=$(( (C_END - C_START) / 1000000 ))  # Convert to milliseconds

C_SIZE=$(ls -lh chat | awk '{print $5}')

echo "Build time: ${C_TIME}ms"
echo "Executable size: ${C_SIZE}"

cd ..
echo ""

# Zig Version Benchmark
echo "Zig Version Benchmark"
echo "------------------------------------------"

cd zig
echo "Cleaning..."
rm -rf zig-out .zig-cache > /dev/null 2>&1
mkdir -p zig-out/bin

echo "Building Zig version..."
ZIG_START=$(date +%s%N)
zig build-exe src/main.zig -femit-bin=zig-out/bin/chat > /dev/null 2>&1
ZIG_END=$(date +%s%N)
ZIG_TIME=$(( (ZIG_END - ZIG_START) / 1000000 ))  # Convert to milliseconds

ZIG_SIZE=$(ls -lh zig-out/bin/chat | awk '{print $5}')

echo "Build time: ${ZIG_TIME}ms"
echo "Executable size: ${ZIG_SIZE}"

cd ..
echo ""

# Comparison
echo "Comparison"
echo "------------------------------------------"
if [ $C_TIME -lt $ZIG_TIME ]; then
  RATIO=$(echo "scale=2; $ZIG_TIME / $C_TIME" | bc)
  echo "C version is ${RATIO}x faster"
else
  RATIO=$(echo "scale=2; $C_TIME / $ZIG_TIME" | bc)
  echo "Zig version is ${RATIO}x faster"
fi

echo ""
echo "Summary:"
echo "  C:   ${C_TIME}ms, ${C_SIZE}"
echo "  Zig: ${ZIG_TIME}ms, ${ZIG_SIZE}"
