#include "qcc5181.h"
#include "dsp_lib.h"
#include <stdio.h>
#include "onnxruntime_c_api.h"

#define SAMPLE_RATE 16000
#define FFT_SIZE 320
#define HOP_SIZE 160
#define MIN_DB -12

// Assuming these are defined somewhere in the QCC5181 SDK
extern void QCC_InitBluetooth();
extern int QCC_ReadAudio(float* buffer, int length);
extern void QCC_BluetoothSend(const char* data, int length);
extern float* calculate_fft(float* input, int length);

void process_audio();

int main(void) {
    // Initialize hardware and Bluetooth on QCC5181
    QCC_InitBluetooth();

    // Start audio processing
    process_audio();

    return 0;
}

void process_audio() {
    float audio_buffer[SAMPLE_RATE]; // Buffer to hold one second of audio
    float* log_powspec;
    int read;

    while (1) {
        // Read audio data from microphone or connected device
        read = QCC_ReadAudio(audio_buffer, SAMPLE_RATE);
        if (read < SAMPLE_RATE) {
            continue; // Handle error or wait for more data
        }

        // Calculate log power spectrum
        log_powspec = calculate_logpowspec(audio_buffer, SAMPLE_RATE);

        // Here you would add the logic to calculate MOS or other metrics
        // For now, let's assume we send this log power spectrum over Bluetooth
        QCC_BluetoothSend((const char*)log_powspec, FFT_SIZE / 2 + 1);

        // Free resources if needed, in real SDK you might have specific functions for this
    }
}

float* calculate_logpowspec(float* audio, int len) {
    float* fft_results = calculate_fft(audio, len);
    float* powspec = (float*)malloc((FFT_SIZE / 2 + 1) * sizeof(float));

    for (int i = 0; i < FFT_SIZE / 2 + 1; ++i) {
        powspec[i] = powf(fft_results[i], 2);
        if (powspec[i] < powf(10, MIN_DB)) {
            powspec[i] = powf(10, MIN_DB);
        }
        powspec[i] = log10f(powspec[i]);
    }

    free(fft_results);
    return powspec;
}


// Define paths to the ONNX model and input data
const char* model_path = "path_to_model.onnx";
const ORTCHAR_T* input_node_names[] = {"input_node"};
const ORTCHAR_T* output_node_names[] = {"output_node"};

int main() {
    OrtEnv* env;
    OrtSession* session;
    OrtSessionOptions* session_options;

    // Initialize environment
    if (OrtCreateEnv(ORT_LOGGING_LEVEL_WARNING, "test", &env) != NULL) {
        printf("Failed to create ONNX runtime environment\n");
        return -1;
    }

    // Create session options
    if (OrtCreateSessionOptions(&session_options) != NULL) {
        OrtReleaseEnv(env);
        printf("Failed to create session options\n");
        return -1;
    }

    // Create session
    if (OrtCreateSession(env, model_path, session_options, &session) != NULL) {
        OrtReleaseSessionOptions(session_options);
        OrtReleaseEnv(env);
        printf("Failed to create session\n");
        return -1;
    }

    // Prepare input tensor
    size_t input_tensor_size = 128;  // Adjust according to your model's expected input
    float* input_tensor_values = (float*)malloc(input_tensor_size * sizeof(float));
    for (int i = 0; i < input_tensor_size; i++) {
        input_tensor_values[i] = (float)i;  // Dummy data; replace with actual data
    }

    // Specify input tensor shape and data
    OrtAllocator* allocator;
    OrtCreateDefaultAllocator(&allocator);
    int64_t input_shape[] = {1, 128};
    OrtValue* input_tensor = NULL;
    if (OrtCreateTensorWithDataAsOrtValue(allocator, input_tensor_values, input_tensor_size * sizeof(float), input_shape, 2, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, &input_tensor) != NULL) {
        printf("Failed to create input tensor\n");
        free(input_tensor_values);
        OrtReleaseAllocator(allocator);
        OrtReleaseSession(session);
        OrtReleaseSessionOptions(session_options);
        OrtReleaseEnv(env);
        return -1;
    }

    // Run model
    OrtValue* output_tensor = NULL;
    if (OrtRun(session, NULL, input_node_names, (const OrtValue* const*)&input_tensor, 1, output_node_names, 1, &output_tensor) != NULL) {
        printf("Failed to run model\n");
    }

    // Retrieve and print the output
    float* output_array;
    OrtGetTensorMutableData(output_tensor, (void**)&output_array);
    printf("Model output: %f\n", output_array[0]);

    // Clean up
    OrtReleaseValue(output_tensor);
    OrtReleaseValue(input_tensor);
    free(input_tensor_values);
    OrtReleaseAllocator(allocator);
    OrtReleaseSession(session);
    OrtReleaseSessionOptions(session_options);
    OrtReleaseEnv(env);

    return 0;
}
