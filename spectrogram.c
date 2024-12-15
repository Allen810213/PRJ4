// spectrogram.c
// Enhanced version to debug and test WAV data processing and ASCII output
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#define PI 3.14159265358979323846

// WAV Header 結構
typedef struct {
    char chunk_id[4];       // "RIFF"
    int chunk_size;         // 文件總大小 - 8
    char format[4];         // "WAVE"
    char subchunk1_id[4];   // "fmt "
    int subchunk1_size;     // PCM 固定為 16
    short audio_format;     // PCM = 1
    short num_channels;     // 通道數
    int sample_rate;        // 採樣率
    int byte_rate;          // ByteRate = SampleRate * NumChannels * BitsPerSample / 8
    short block_align;      // BlockAlign = NumChannels * BitsPerSample / 8
    short bits_per_sample;  // 每個樣本的位深
    char subchunk2_id[4];   // "data"
    int subchunk2_size;     // 音訊數據的大小
} WAVHeader;

// 窗口函數
void apply_window(double *windowed_signal, short *raw_signal, int window_size, const char *window_type) {
    for (int i = 0; i < window_size; i++) {
        if (strcmp(window_type, "hamming") == 0) {
            double hamming_coeff = 0.54 - 0.46 * cos(2 * PI * i / (window_size - 1));
            windowed_signal[i] = raw_signal[i] * hamming_coeff;
        } else if (strcmp(window_type, "hanning") == 0) {
            double hanning_coeff = 0.5 * (1 - cos(2 * PI * i / (window_size - 1)));
            windowed_signal[i] = raw_signal[i] * hanning_coeff;
        } else {
            windowed_signal[i] = raw_signal[i]; // Rectangular (default)
        }
    }
}

// FFT 計算（簡單實現）
void compute_fft(const double *signal, double *spectrum, int size) {
    for (int k = 0; k < size; k++) {
        double real = 0.0, imag = 0.0;
        for (int n = 0; n < size; n++) {
            double angle = 2 * PI * k * n / size;
            real += signal[n] * cos(angle);
            imag -= signal[n] * sin(angle);
        }
        spectrum[k] = 20 * log10(sqrt(real * real + imag * imag) + 1e-10); // dB scale
    }
}

// 主數據處理
void process_wav(const char *wav_file, const char *output_file, int w_size_ms, const char *w_type, int dft_size, int f_itv_ms) {
    FILE *input = fopen(wav_file, "rb");
    if (!input) {
        perror("Error opening input WAV file");
        exit(1);
    }

    FILE *output = fopen(output_file, "w");
    if (!output) {
        perror("Error opening output TXT file");
        fclose(input);
        exit(1);
    }

    WAVHeader header;
    fread(&header, sizeof(WAVHeader), 1, input);

    if (strncmp(header.chunk_id, "RIFF", 4) != 0 || strncmp(header.format, "WAVE", 4) != 0) {
        fprintf(stderr, "Error: Not a valid WAV file.\n");
        fclose(input);
        fclose(output);
        exit(1);
    }

    printf("Sample rate: %d, Channels: %d, Subchunk2 size: %d\n", header.sample_rate, header.num_channels, header.subchunk2_size);

    if (header.num_channels != 1 || header.bits_per_sample != 16) {
        fprintf(stderr, "Error: Only mono 16-bit WAV files are supported.\n");
        fclose(input);
        fclose(output);
        exit(1);
    }

    int sample_rate = header.sample_rate;
    int window_size = (w_size_ms * sample_rate) / 1000;
    int frame_interval = (f_itv_ms * sample_rate) / 1000;
    int num_samples = header.subchunk2_size / sizeof(short);

    printf("Total samples: %d, Window size: %d, Frame interval: %d\n", num_samples, window_size, frame_interval);

    short *buffer = (short *)malloc(sizeof(short) * window_size);
    double *windowed_signal = (double *)malloc(sizeof(double) * window_size);
    double *spectrum = (double *)malloc(sizeof(double) * (dft_size / 2 + 1));

    if (!buffer || !windowed_signal || !spectrum) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(input);
        fclose(output);
        free(buffer);
        free(windowed_signal);
        free(spectrum);
        exit(1);
    }

    int frame_count = 0;
    while (fread(buffer, sizeof(short), window_size, input) == window_size) {
        printf("Processing frame %d with %d samples\n", frame_count, window_size);

        for (int i = 0; i < 10; i++) {
            printf("Raw Signal[%d]: %d\n", i, buffer[i]);
        }

        apply_window(windowed_signal, buffer, window_size, w_type);

        for (int i = 0; i < 10; i++) {
            printf("Windowed Signal[%d]: %.2f\n", i, windowed_signal[i]);
        }

        compute_fft(windowed_signal, spectrum, dft_size);

        for (int k = 0; k <= dft_size / 2; k++) {
            fprintf(output, "%.2f ", spectrum[k]);
            printf("Spectrum[%d]: %.2f\n", k, spectrum[k]);
        }
        fprintf(output, "\n");
        fflush(output);

        fseek(input, -((window_size - frame_interval) * sizeof(short)), SEEK_CUR);
        frame_count++;
    }

    printf("Total frames processed: %d\n", frame_count);

    fclose(input);
    fclose(output);
    free(buffer);
    free(windowed_signal);
    free(spectrum);

    printf("Spectrogram saved to %s\n", output_file);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: %s <w_size> <w_type> <dft_size> <f_itv> <wav_in> <spec_out>\n", argv[0]);
        return 1;
    }

    int w_size = atoi(argv[1]);
    const char *w_type = argv[2];
    int dft_size = atoi(argv[3]);
    int f_itv = atoi(argv[4]);
    const char *wav_in = argv[5];
    const char *spec_out = argv[6];

    process_wav(wav_in, spec_out, w_size, w_type, dft_size, f_itv);
    return 0;
}

