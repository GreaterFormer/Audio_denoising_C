#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <sndfile.h>
#include <pthread.h>

#define MAX_THREADS 4

typedef struct {
    char *file_name;
    int sample_rate;
    char *dtype;
    int compression;
} AudioData;

void *process_audio(void *arg) {
    AudioData *audio_data = (AudioData *)arg;
    SNDFILE *file;
    SF_INFO info;
    float *buffer;
    hid_t file_id, dataset_id, dataspace_id;  // HDF5 identifiers
    hsize_t dims[1];

    file = sf_open(audio_data->file_name, SFM_READ, &info);
    if (file == NULL) {
        fprintf(stderr, "Failed to open the file %s\n", audio_data->file_name);
        pthread_exit(NULL);
    }

    buffer = malloc(info.frames * sizeof(float) * info.channels);
    sf_read_float(file, buffer, info.frames * info.channels);
    sf_close(file);

    // HDF5 operations
    file_id = H5Fcreate("output.hdf5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    dims[0] = info.frames;
    dataspace_id = H5Screate_simple(1, dims, NULL);
    dataset_id = H5Dcreate(file_id, "dataset", H5T_IEEE_F32LE, dataspace_id,
                           H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
    H5Dclose(dataset_id);
    H5Sclose(dataspace_id);
    H5Fclose(file_id);

    free(buffer);
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    pthread_t threads[MAX_THREADS];
    AudioData audio_data[MAX_THREADS];
    int i;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <audio_file1> <audio_file2> ... \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (i = 1; i < argc && i <= MAX_THREADS; i++) {
        audio_data[i - 1].file_name = argv[i];
        audio_data[i - 1].sample_rate = 48000;  // Example sample rate
        audio_data[i - 1].dtype = "float32";
        audio_data[i - 1].compression = 0;  // Example compression

        if (pthread_create(&threads[i - 1], NULL, process_audio, (void *)&audio_data[i - 1])) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }

    for (i = 0; i < argc - 1 && i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
