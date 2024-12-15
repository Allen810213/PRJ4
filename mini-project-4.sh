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

# Parameters for each Set
declare -A set_params
set_params["Set1"]="32 rectangular 32 10"
set_params["Set2"]="32 hamming 32 10"
set_params["Set3"]="30 rectangular 32 10"
set_params["Set4"]="30 hamming 32 10"

# Generate 16 ASCII spectrogram files
for wav in "${wav_files[@]}"; do
    for set in "${sets[@]}"; do
        # Create the output ASCII filename
        ascii_file="${wav%.wav}.${set}.txt"

        # Get the parameters for this Set
        params=${set_params[$set]}

        # Debug: Print current operation
        echo "Generating spectrogram for $wav with $set -> $ascii_file using params: $params"

        # Run spectrogram program with all required parameters
        ./spectrogram $params "$wav" "$ascii_file" 2>>error.log

        # Check if output file was generated
        if [[ -f "$ascii_file" ]]; then
            echo "Generated: $ascii_file"
        else
            echo "Failed to generate: $ascii_file. Check error.log for details."
        fi
    done
done

# Debug: List all generated ASCII files
echo "Generated ASCII spectrogram files:"
ls -lh *.txt

# Generate PDF files from spectrogram data
for wav in "${wav_files[@]}"; do
    for set in "${sets[@]}"; do
        # Determine input ASCII file and output PDF file
        ascii_file="${wav%.wav}.${set}.txt"
        pdf_file="${wav%.wav}.${set}.pdf"

        # Debug: Print current operation
        echo "Generating PDF for $wav with $set -> $pdf_file"

        # Run spectshow.py to generate PDF
        python3 spectshow.py "$wav" "$ascii_file" "$pdf_file"

        # Check if PDF was generated
        if [[ -f "$pdf_file" ]]; then
            echo "Generated: $pdf_file"
        else
            echo "Failed to generate: $pdf_file"
        fi
    done
done

# Debug: List all generated PDF files
echo "Generated PDF files:"
ls -lh *.pdf

