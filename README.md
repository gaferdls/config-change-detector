# Build Config Change Detector

A systems tool that monitors configuration files, detects unauthorized changes between runs, and generates a structured audit trail — modeled after integrity checks in safety-critical software environments (DO-178C).

## What It Does

- Computes MD5 checksums for all files in a monitored config directory
- Stores a baseline snapshot on first run
- On subsequent runs, diffs current state against baseline
- Flags any modified or missing files as FAIL
- Generates a human-readable audit report with timestamps and checksums

## Project Structure
```
config-change-detector/
├── configs/          # Monitored configuration files
├── logs/             # snapshot.log and diff.log (generated)
├── reports/          # audit_report.txt (generated)
├── src/
│   ├── checksum_scanner.c   # Scans configs, writes baseline snapshot
│   ├── change_detector.c    # Diffs current state against baseline
│   └── audit_report.py      # Generates human-readable report
└── run_detector.sh          # Entry point
```

## Requirements

- Docker
- Debian container with `gcc` and `python3`

## Usage

Since the scripts expect to run from `/workspace`, first start a Debian container and mount your current directory:

```bash
docker run -it -v $(pwd):/workspace debian:latest bash

# Inside the container, install the required packages:
apt-get update && apt-get install -y gcc python3
cd /workspace
```

Build the binaries inside the container:

```bash
gcc src/checksum_scanner.c -o src/checksum_scanner
gcc src/change_detector.c -o src/change_detector
```

Take a baseline snapshot:

```bash
./run_detector.sh baseline
```

Check for changes:

```bash
./run_detector.sh check
```

## Example Output

```text
[1/2] Detecting changes...
FAIL: app.conf -> CHANGED
PASS: build.conf
PASS: release.conf
Total changes: 1
```


## Why It Matters

In avionics and safety-critical software (DO-178C), unauthorized config changes can invalidate an entire certification baseline. This tool simulates that guardrail by enforcing checksum-based integrity checks on every build.