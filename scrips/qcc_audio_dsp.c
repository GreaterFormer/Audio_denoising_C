#include "qcc5181.h"
#include "dsp_lib.h"

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
