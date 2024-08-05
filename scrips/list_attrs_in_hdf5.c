#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <hdf5.h>
#include <stdbool.h>
#include <string.h>

void process_file(const char* filename, bool show_keys, bool calculate_hours) {
    hid_t file_id, dataset_id, space_id, attr_id;
    herr_t status;
    H5T_class_t class;
    size_t size;

    file_id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return;
    }

    // Reading attributes
    int n_attrs = H5Aget_num_attrs(file_id);
    for (int i = 0; i < n_attrs; i++) {
        attr_id = H5Aopen_idx(file_id, i);
        H5Aget_info(attr_id, &info);
        char attr_name[1024];
        H5Aget_name(attr_id, sizeof(attr_name), attr_name);
        printf("Found attribute %s\n", attr_name);
        H5Aclose(attr_id);
    }

    // Iterate over datasets here, similar to handling attributes
    // Skipping for brevity

    H5Fclose(file_id);
}

int main(int argc, char** argv) {
    int option;
    bool keys = false, hours = false;

    while ((option = getopt(argc, argv, "kh")) != -1) {
        switch (option) {
            case 'k':
                keys = true;
                break;
            case 'h':
                hours = true;
                break;
            default:
                fprintf(stderr, "Usage: %s [-k] [-h] file1 [file2 ...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    for (int i = optind; i < argc; i++) {
        process_file(argv[i], keys, hours);
    }

    return 0;
}
