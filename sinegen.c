// sinegen.c
// 用於生成短音訊（WAV 文件）並自動生成 SCP 文件
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#define PI 3.14159265358979323846

// 單位步階函數
double step_function(double t) {
    return (t >= 0) ? 1.0 : 0.0;
}

// 波形生成函數
double generate_waveform(int j, double t, double f) {
    switch (j) {
        case 0: // Sine wave
            return sin(2 * PI * f * t);
        case 1: // Sawtooth wave
            return f * t - floor(f * t + 0.5);
        case 2: // Square wave
            return (sin(2 * PI * f * t) > 0) ? 1.0 : -1.0;
        case 3: // Triangle wave
            return 2.0 * fabs(2.0 * (f * t - floor(f * t + 0.5))) - 1.0;
        default:
            return 0.0;
    }
}

// 生成短音訊 WAV 文件
void generate_wave(const char *filename, int sample_rate, int duration_ms, int j, int i, double amplitude, double frequency) {
    int num_samples = (sample_rate * duration_ms) / 1000;
    int16_t *buffer = (int16_t *)malloc(sizeof(int16_t) * num_samples);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    for (int n = 0; n < num_samples; n++) {
        double t = (double)n / sample_rate; // 當前時間
        double u_start = step_function(t);
        double u_stop = step_function(t - 0.1);
        double s = generate_waveform(j, t, frequency);
        buffer[n] = (int16_t)(amplitude * s * (u_start - u_stop));
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Could not open file %s for writing.\n", filename);
        free(buffer);
        exit(1);
    }

    // Write WAV header
    fwrite("RIFF", 1, 4, file);
    int chunk_size = 36 + num_samples * 2;
    fwrite(&chunk_size, 4, 1, file);
    fwrite("WAVEfmt ", 1, 8, file);
    int subchunk1_size = 16;
    short audio_format = 1;
    short num_channels = 1;
    fwrite(&subchunk1_size, 4, 1, file);
    fwrite(&audio_format, 2, 1, file);
    fwrite(&num_channels, 2, 1, file);
    fwrite(&sample_rate, 4, 1, file);
    int byte_rate = sample_rate * num_channels * sizeof(int16_t);
    short block_align = num_channels * sizeof(int16_t);
    short bits_per_sample = 16;
    fwrite(&byte_rate, 4, 1, file);
    fwrite(&block_align, 2, 1, file);
    fwrite(&bits_per_sample, 2, 1, file);
    fwrite("data", 1, 4, file);
    int data_chunk_size = num_samples * 2;
    fwrite(&data_chunk_size, 4, 1, file);

    // Write audio data
    fwrite(buffer, sizeof(int16_t), num_samples, file);

    fclose(file);
    free(buffer);

    printf("Generated WAV file: %s\n", filename);
}

int main() {
    const int sample_rates[] = {8000, 16000};
    const char *wave_types[] = {"sine", "sawtooth", "square", "triangle"};
    const double frequencies[] = {0, 31.25, 500, 2000, 4000, 44, 220, 440, 1760, 3960};
    const double amplitudes[] = {100, 2000, 1000, 500, 250, 100, 2000, 1000, 500, 250};

    FILE *scp_8k = fopen("8k.scp", "w");
    FILE *scp_16k = fopen("16k.scp", "w");
    if (!scp_8k || !scp_16k) {
        fprintf(stderr, "Could not open SCP files for writing.\n");
        exit(1);
    }

    for (int sr = 0; sr < 2; sr++) { // 遍歷兩種採樣率
        for (int j = 0; j < 4; j++) { // 遍歷波形類型
            for (int i = 0; i < 10; i++) { // 遍歷頻率和振幅組合
                char filename[256];
                snprintf(filename, sizeof(filename), "wave_%dk_%s_f%.0f_a%.0f.wav", sample_rates[sr] / 1000, wave_types[j], frequencies[i], amplitudes[i]);
                generate_wave(filename, sample_rates[sr], 100, j, i, amplitudes[i], frequencies[i]);
                printf("Generated: %s\n", filename);

                if (sample_rates[sr] == 8000) {
                    fprintf(scp_8k, "%s\n", filename);
                } else if (sample_rates[sr] == 16000) {
                    fprintf(scp_16k, "%s\n", filename);
                }
            }
        }
    }

    fclose(scp_8k);
    fclose(scp_16k);

    printf("SCP files generated: 8k.scp and 16k.scp\n");
    return 0;
}

