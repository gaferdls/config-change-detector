#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CONFIG_DIR "/workspace/configs"
#define SNAPSHOT_FILE "/workspace/logs/snapshot.log"
#define MAX_PATH 512
#define CHECKSUM_LEN 64

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

int main() {
  DIR *dir = opendir(CONFIG_DIR);
  if (!dir) {
    fprintf(stderr, "ERROR: Cannot open config directory: %s\n", CONFIG_DIR);
    return 1;
  }
  FILE *snapshot = fopen(SNAPSHOT_FILE, "w");
  if (!snapshot) {
    perror("fopen failed");
    fprintf(stderr, "ERROR: Cannot write snapshot file: %s\n", SNAPSHOT_FILE);
    closedir(dir);
    return 1;
  }

  time_t now = time(NULL);
  fprintf(snapshot, "# Snapshot timestamp: %s", ctime(&now));

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_name[0] == '.')
      continue;

    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "%s/%s", CONFIG_DIR, entry->d_name);

    char checksum[CHECKSUM_LEN];
    compute_checksum(filepath, checksum);

    fprintf(snapshot, "%s %s\n", checksum, entry->d_name);
    printf("Scanned: %s -> %s\n", entry->d_name, checksum);
  }

  closedir(dir);
  fclose(snapshot);
  printf("Snapshot written to %s\n", SNAPSHOT_FILE);
  return 0;
}