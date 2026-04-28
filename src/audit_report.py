import os
from datetime import datetime

DIFF_LOG = "/workspace/logs/diff.log"
REPORT_FILE = "/workspace/reports/audit_report.txt"

def parse_diff_log(filepath):
    records = []
    timestamp = None
    total_changes = 0

    with open(filepath, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            if line.startswith("# Diff report timestamp:"):
                timestamp = line.replace("# Diff report timestamp:", "").strip()

            elif line.startswith("# Total changes detected:"):
                total_changes = int(line.split(":")[1].strip())

            elif line.startswith("FAIL") or line.startswith("PASS"):
                parts = line.split()
                status = parts[0]
                filename = parts[1]

                if status == "FAIL" and "FILE MISSING" in line:
                    records.append({
                        "status": status,
                        "filename": filename,
                        "baseline": "N/A",
                        "current": "FILE MISSING"
                    })
                elif status == "FAIL":
                    baseline = parts[2].replace("baseline=", "")
                    current = parts[3].replace("current=", "")
                    records.append({
                        "status": status,
                        "filename": filename,
                        "baseline": baseline,
                        "current": current
                    })
                else:
                    records.append({
                        "status": status,
                        "filename": filename,
                        "baseline": parts[2],
                        "current": parts[2]
                    })

    return timestamp, total_changes, records


def write_report(timestamp, total_changes, records):
    os.makedirs(os.path.dirname(REPORT_FILE), exist_ok=True)

    with open(REPORT_FILE, "w") as f:
        f.write("=" * 60 + "\n")
        f.write("       BUILD CONFIG INTEGRITY AUDIT REPORT\n")
        f.write("=" * 60 + "\n")
        f.write(f"  Scan Time   : {timestamp}\n")
        f.write(f"  Report Gen  : {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write(f"  Total Files : {len(records)}\n")
        f.write(f"  Changes     : {total_changes}\n")
        overall = "FAIL" if total_changes > 0 else "PASS"
        f.write(f"  Status      : {overall}\n")
        f.write("=" * 60 + "\n\n")

        f.write("FILE RESULTS\n")
        f.write("-" * 60 + "\n")

        for r in records:
            f.write(f"  [{r['status']}] {r['filename']}\n")
            if r["status"] == "FAIL":
                f.write(f"         Baseline : {r['baseline']}\n")
                f.write(f"         Current  : {r['current']}\n")
        
        f.write("\n" + "-" * 60 + "\n")

        if total_changes == 0:
            f.write("  RESULT: All config files match baseline. No action required.\n")
        else:
            f.write(f"  RESULT: {total_changes} file(s) modified. Investigate before build.\n")

        f.write("=" * 60 + "\n")


if __name__ == "__main__":
    timestamp, total_changes, records = parse_diff_log(DIFF_LOG)
    write_report(timestamp, total_changes, records)
    print(f"Audit report written to {REPORT_FILE}")