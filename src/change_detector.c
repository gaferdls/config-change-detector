#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CONFIG_DIR "/workspace/configs"
#define SNAPSHOT_FILE "/workspace/logs/snapshot.log"
#define DIFF_LOG "/workspace/logs/diff.log"
#define MAX_PATH 512
#define CHECKSUM_LEN 64
#define FILENAME_LEN 256
#define MAX_FILES 64

typedef struct {
  char filename[FILENAME_LEN];
  char checksum[CHECKSUM_LEN];
} FileRecord;

void compute_checksum(const char *filepath, char *out_checksum) {
  char command[MAX_PATH + 32];
  snprintf(command, sizeof(command), "md5sum %s", filepath);

  FILE *fp = popen(command, "r");
  if (!fp) {
    strcpy(out_checksum, "ERROR");
    return;
  }

  fscanf(fp, "%63s", out_checksum);
  pclose(fp);
}

int load_snapshot(FileRecord *records, int *count) {
  FILE *fp = fopen(SNAPSHOT_FILE, "r");
  if (!fp) {
    fprintf(stderr, "ERROR: Cannot open snapshot: %s\n", SNAPSHOT_FILE);
    return 0;
  }

  char line[MAX_PATH];
  *count = 0;

  while (fgets(line, sizeof(line), fp)) {
    if (line[0] == '#')
      continue;
    if (sscanf(line, "%63s %255s", records[*count].checksum,
               records[*count].filename) == 2) {
      (*count)++;
    }
  }

  fclose(fp);
  return 1;
}

int find_record(FileRecord *records, int count, const char *filename) {
  for (int i = 0; i < count; i++) {
    if (strcmp(records[i].filename, filename) == 0)
      return i;
  }
  return -1;
}

int main() {
  FileRecord baseline[MAX_FILES];
  int baseline_count = 0;

  if (!load_snapshot(baseline, &baseline_count))
    return 1;

  FILE *diff = fopen(DIFF_LOG, "w");
  if (!diff) {
    perror("fopen diff log failed");
    return 1;
  }

  time_t now = time(NULL);
  fprintf(diff, "# Diff report timestamp: %s", ctime(&now));

  int changes = 0;

  for (int i = 0; i < baseline_count; i++) {
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "%s/%s", CONFIG_DIR,
             baseline[i].filename);

    char current_checksum[CHECKSUM_LEN];
    compute_checksum(filepath, current_checksum);

    if (strcmp(current_checksum, "ERROR") == 0) {
      fprintf(diff, "FAIL  %s  [FILE MISSING]\n", baseline[i].filename);
      printf("FAIL: %s -> FILE MISSING\n", baseline[i].filename);
      changes++;
    } else if (strcmp(current_checksum, baseline[i].checksum) != 0) {
      fprintf(diff, "FAIL  %s  baseline=%s  current=%s\n", baseline[i].filename,
              baseline[i].checksum, current_checksum);
      printf("FAIL: %s -> CHANGED\n", baseline[i].filename);
      changes++;
    } else {
      fprintf(diff, "PASS  %s  %s\n", baseline[i].filename,
              baseline[i].checksum);
      printf("PASS: %s\n", baseline[i].filename);
    }
  }

  fprintf(diff, "# Total changes detected: %d\n", changes);
  fclose(diff);

  printf("Diff log written to %s\n", DIFF_LOG);
  printf("Total changes: %d\n", changes);
  return 0;
}