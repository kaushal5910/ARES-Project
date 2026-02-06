# ARES Project - Quick Start Guide

## Build the Project

```powershell
.\build.ps1
```

## Run Everything

### 1. Run Tests (Verify Correctness)
```bash
.\build\test_aes.exe
.\build\test_gaussian.exe
```

### 2. Run Benchmarks (Measure Performance)
```bash
.\build\bench_aes.exe
.\build\bench_gaussian.exe
```

### 3. Run Demo (See Visual Results!)
```bash
# Run both demos
.\build\ares_demo.exe

# Just blur (creates images)
.\build\ares_demo.exe blur

# Just AES (shows speedup)
.\build\ares_demo.exe aes
```

## View Results

**Benchmark numbers:** Printed in terminal

**Blurred images:** Open the `.ppm` files:
- `original.ppm`
- `blurred_baseline.ppm`
- `blurred_simd.ppm`
- `blurred_tiled.ppm`

**How to open PPM:**
- Double-click (Windows Photos)
- Right-click → Open with Browser
- Convert online: https://convertio.co/ppm-png/

## Create Performance Charts

```bash
pip install matplotlib numpy
python visualize_performance.py
```

Opens: `performance_charts.png` and `speedup_summary.png`

## Key Results

✅ **AES-NI Speedup:** 67-123x faster encryption  
✅ **Visual Proof:** Actual blurred images generated  
✅ **Professional Demo:** One-command demonstration ready  

Perfect for internship applications! 🚀
