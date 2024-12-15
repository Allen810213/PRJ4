import matplotlib.pyplot as plt
import numpy as np
import wave
import sys

def read_wav_file(wav_file):
    """Read WAV file and return sample rate and audio data."""
    with wave.open(wav_file, 'r') as wav:
        sample_rate = wav.getframerate()
        num_frames = wav.getnframes()
        audio_data = np.frombuffer(wav.readframes(num_frames), dtype=np.int16)
    return sample_rate, audio_data

def read_spectrogram_file(spectrogram_file):
    """Read ASCII spectrogram data file and return 2D numpy array."""
    with open(spectrogram_file, 'r') as f:
        spectrogram_data = [list(map(float, line.strip().split())) for line in f]
    return np.array(spectrogram_data)

def plot_waveform(sample_rate, audio_data, ax):
    """Plot waveform on the provided axes."""
    time_axis = np.linspace(0, len(audio_data) / sample_rate, num=len(audio_data))
    ax.plot(time_axis, audio_data, color='blue')
    ax.set_title("Waveform")
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Amplitude")

def plot_spectrogram(spectrogram_data, ax):
    """Plot spectrogram on the provided axes."""
    ax.imshow(spectrogram_data, aspect='auto', origin='lower', cmap='viridis', extent=[0, spectrogram_data.shape[1], 0, spectrogram_data.shape[0]])
    ax.set_title("Spectrogram")
    ax.set_xlabel("Frequency Bins")
    ax.set_ylabel("Time Frames")

def generate_pdf(wav_file, spectrogram_file, output_pdf):
    """Generate a PDF with waveform and spectrogram plots."""
    sample_rate, audio_data = read_wav_file(wav_file)
    spectrogram_data = read_spectrogram_file(spectrogram_file)

    fig, axs = plt.subplots(2, 1, figsize=(8, 10))
    plot_waveform(sample_rate, audio_data, axs[0])
    plot_spectrogram(spectrogram_data, axs[1])

    plt.tight_layout()
    plt.savefig(output_pdf)
    plt.close()
    print(f"Generated PDF: {output_pdf}")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python3 spectshow.py <in_wav> <in_txt> <out_pdf>")
        sys.exit(1)

    wav_file = sys.argv[1]
    spectrogram_file = sys.argv[2]
    output_pdf = sys.argv[3]

    try:
        generate_pdf(wav_file, spectrogram_file, output_pdf)
    except Exception as e:
        print(f"Error: {e}")

