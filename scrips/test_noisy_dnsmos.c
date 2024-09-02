#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    char *dataset_dir = NULL;
    int v1 = 0, v5 = 0, metric_workers = 4;
    char *csv_path_enh = NULL, *csv_path_noisy = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--v1") == 0) {
            v1 = 1;
        } else if (strcmp(argv[i], "--v5") == 0) {
            v5 = 1;
        } else if (strcmp(argv[i], "--metric-workers") == 0 && i + 1 < argc) {
            metric_workers = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--csv-path-enh") == 0 && i + 1 < argc) {
            csv_path_enh = argv[++i];
        } else if (strcmp(argv[i], "--csv-path-noisy") == 0 && i + 1 < argc) {
            csv_path_noisy = argv[++i];
        } else {
            dataset_dir = argv[i];
        }
    }

    if (dataset_dir == NULL) {
        fprintf(stderr, "Dataset directory not specified\n");
        return 1;
    }

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(dataset_dir)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            printf("%s\n", ent->d_name);
        }
        closedir(dir);
    } else {
        perror("");
        return EXIT_FAILURE;
    }

    // Further processing and memory cleanup would go here

    return 0;
}
