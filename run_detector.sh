#!/bin/bash

set -e

echo "=== Config Change Detector ==="
echo ""

if [ "$1" == "baseline" ]; then
    echo "[BASELINE] Scanning config files..."
    /workspace/src/checksum_scanner
    echo "Baseline snapshot saved."

elif [ "$1" == "check" ]; then
    echo "[1/2] Detecting changes..."
    /workspace/src/change_detector

    echo ""
    echo "[2/2] Generating audit report..."
    python3 /workspace/src/audit_report.py

    echo ""
    echo "=== Done. Report at /workspace/reports/audit_report.txt ==="

else
    echo "Usage: run_detector.sh [baseline|check]"
    exit 1
fi