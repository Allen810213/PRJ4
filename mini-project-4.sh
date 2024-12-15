#!/bin/bash

# Compile all C programs
gcc -o sinegen sinegen.c -lm
gcc -o cascade cascade.c
gcc -o spectrogram spectrogram.c -lm

# Generate short audio files
./sinegen

# Cascade short audio files into s-8k.wav and s-16k.wav
./cascade 8k.scp s-8k.wav
./cascade 16k.scp s-16k.wav

# Debug: Check if input WAV files exist
echo "Checking existence of WAV files..."
ls -lh aeueo-8kHz.wav aeueo-16kHz.wav s-8k.wav s-16k.wav || { echo "Some WAV files are missing."; exit 1; }

# Define input WAV files and their corresponding output ASCII files
declare -a wav_files=("aeueo-8kHz.wav" "aeueo-16kHz.wav" "s-8k.wav" "s-16k.wav")
declare -a sets=("Set1" "Set2" "Set3" "Set4")

# Generate 16 ASCII spectrogram files
for wav in "${wav_files[@]}"; do
    for set in "${sets[@]}"; do
        # Create the output ASCII filename
        ascii_file="${wav%.wav}.${set}.txt"

        # Debug: Print current operation
        echo "Generating spectrogram for $wav -> $ascii_file"

        # Run spectrogram program
        if [[ "$set" == "Set1" ]]; then
            ./spectrogram 32 0 32 10 "$wav" "$ascii_file"
        elif [[ "$set" == "Set2" ]]; then
            ./spectrogram 32 1 32 10 "$wav" "$ascii_file"
        elif [[ "$set" == "Set3" ]]; then
            ./spectrogram 30 0 32 10 "$wav" "$ascii_file"
        elif [[ "$set" == "Set4" ]]; then
            ./spectrogram 30 1 32 10 "$wav" "$ascii_file"
        else
            echo "Unknown set: $set"; exit 1;
        fi

        # Check if output file was generated
        if [[ -f "$ascii_file" ]]; then
            echo "Generated: $ascii_file"
        else
            echo "Failed to generate: $ascii_file"; exit 1;
        fi
    done
done

# Generate 16 PDF files using spectshow.py
for wav in "${wav_files[@]}"; do
    for set in "${sets[@]}"; do
        ascii_file="${wav%.wav}.${set}.txt"
        pdf_file="${wav%.wav}.${set}.pdf"

        # Debug: Print current operation
        echo "Generating PDF for $ascii_file -> $pdf_file"

        # Run spectshow.py to generate PDF
        python3 spectshow.py "$wav" "$ascii_file" "$pdf_file" || { echo "Error generating $pdf_file"; exit 1; }

        # Check if PDF file was generated
        if [[ -f "$pdf_file" ]]; then
            echo "Generated: $pdf_file"
        else
            echo "Failed to generate: $pdf_file"; exit 1;
        fi
    done
done

# Debug: List all generated files
echo "Generated ASCII spectrogram files:"
ls -lh *.txt
echo "Generated PDF files:"
ls -lh *.pdf

