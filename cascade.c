// cascade.c
// 用於合併多個短音訊文件成一個長音訊文件，根據 SCP 文件指定
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// WAV Header 結構
typedef struct {
    char chunk_id[4];       // "RIFF"
    int chunk_size;         // 36 + SubChunk2Size
    char format[4];         // "WAVE"
    char subchunk1_id[4];   // "fmt "
    int subchunk1_size;     // 16 for PCM
    short audio_format;     // PCM = 1
    short num_channels;     // 1
    int sample_rate;        // 8000 or 16000
    int byte_rate;          // SampleRate * NumChannels * BitsPerSample/8
    short block_align;      // NumChannels * BitsPerSample/8
    short bits_per_sample;  // 16
    char subchunk2_id[4];   // "data"
    int subchunk2_size;     // NumSamples * NumChannels * BitsPerSample/8
} WAVHeader;

void merge_wav(const char *scp_file, const char *output_file) {
    FILE *scp = fopen(scp_file, "r");
    if (!scp) {
        fprintf(stderr, "Error: Could not open SCP file %s.\n", scp_file);
        exit(1);
    }

    FILE *output = fopen(output_file, "wb");
    if (!output) {
        fprintf(stderr, "Error: Could not create output file %s.\n", output_file);
        fclose(scp);
        exit(1);
    }

    WAVHeader header;
    memset(&header, 0, sizeof(WAVHeader));

    char input_filename[256];
    int total_data_size = 0;
    int sample_rate = 0;
    int num_channels = 0;
    int bits_per_sample = 0;

    // Reserve space for the output header
    fwrite(&header, sizeof(WAVHeader), 1, output);

    while (fgets(input_filename, sizeof(input_filename), scp)) {
        // Remove newline character
        input_filename[strcspn(input_filename, "\n")] = 0;

        FILE *input = fopen(input_filename, "rb");
        if (!input) {
            fprintf(stderr, "Error: Could not open input file %s. Skipping.\n", input_filename);
            continue;
        }

        WAVHeader input_header;
        fread(&input_header, sizeof(WAVHeader), 1, input);

        if (sample_rate == 0) {
            sample_rate = input_header.sample_rate;
            num_channels = input_header.num_channels;
            bits_per_sample = input_header.bits_per_sample;
        } else if (sample_rate != input_header.sample_rate ||
                   num_channels != input_header.num_channels ||
                   bits_per_sample != input_header.bits_per_sample) {
            fprintf(stderr, "Error: Input file %s has incompatible format. Skipping.\n", input_filename);
            fclose(input);
            continue;
        }

        int data_size = input_header.subchunk2_size;
        total_data_size += data_size;

        short buffer[1024];
        int read;
        while ((read = fread(buffer, sizeof(short), 1024, input)) > 0) {
            fwrite(buffer, sizeof(short), read, output);
        }

        fclose(input);
    }

    // Fill the WAV header
    strncpy(header.chunk_id, "RIFF", 4);
    header.chunk_size = 36 + total_data_size;
    strncpy(header.format, "WAVE", 4);
    strncpy(header.subchunk1_id, "fmt ", 4);
    header.subchunk1_size = 16;
    header.audio_format = 1;
    header.num_channels = num_channels;
    header.sample_rate = sample_rate;
    header.byte_rate = sample_rate * num_channels * bits_per_sample / 8;
    header.block_align = num_channels * bits_per_sample / 8;
    header.bits_per_sample = bits_per_sample;
    strncpy(header.subchunk2_id, "data", 4);
    header.subchunk2_size = total_data_size;

    // Write the header to the output file
    fseek(output, 0, SEEK_SET);
    fwrite(&header, sizeof(WAVHeader), 1, output);

    fclose(output);
    fclose(scp);

    printf("Merged WAV file created: %s\n", output_file);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <scp_file> <output_file>\n", argv[0]);
        return 1;
    }

    merge_wav(argv[1], argv[2]);
    return 0;
}

