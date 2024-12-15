import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import wave
import sys

def plot_waveform(wav_file, ax):
    """
    繪製波形圖。
    :param wav_file: 輸入的 WAV 文件名
    :param ax: Matplotlib 的軸對象
    """
    with wave.open(wav_file, 'r') as wav:
        n_frames = wav.getnframes()
        framerate = wav.getframerate()
        data = wav.readframes(n_frames)

        # 將數據轉換為 NumPy 陣列
        waveform = np.frombuffer(data, dtype=np.int16)
        time = np.linspace(0, n_frames / framerate, num=n_frames)

        ax.plot(time, waveform, color='blue')
        ax.set_title(f"Waveform of {wav_file}")
        ax.set_xlabel("Time (s)")
        ax.set_ylabel("Amplitude")

def plot_spectrogram(txt_file, ax):
    """
    繪製頻譜圖。
    :param txt_file: 輸入的 ASCII 頻譜數據文件
    :param ax: Matplotlib 的軸對象
    """
    try:
        # 加載 ASCII 文件並轉換為 NumPy 陣列
        spectrum = np.loadtxt(txt_file)
        ax.imshow(spectrum.T, aspect='auto', origin='lower', cmap='viridis')
        ax.set_title(f"Spectrogram of {txt_file}")
        ax.set_xlabel("Time Frame")
        ax.set_ylabel("Frequency Bin")

        # 添加顏色條
        cbar = plt.colorbar(ax.images[0], ax=ax, orientation='vertical')
        cbar.set_label("Amplitude (dB)")
    except Exception as e:
        print(f"Error loading spectrogram data from {txt_file}: {e}")
        sys.exit(1)

def main():
    if len(sys.argv) != 4:
        print("Usage: python3 spectshow.py <in_wav> <in_txt> <out_pdf>")
        sys.exit(1)

    in_wav = sys.argv[1]
    in_txt = sys.argv[2]
    out_pdf = sys.argv[3]

    # 創建 PDF 文件
    with PdfPages(out_pdf) as pdf:
        fig, axs = plt.subplots(2, 1, figsize=(10, 8))

        # 繪製波形圖
        plot_waveform(in_wav, axs[0])

        # 繪製頻譜圖
        plot_spectrogram(in_txt, axs[1])

        # 調整布局並保存
        plt.tight_layout()
        pdf.savefig(fig)
        plt.close(fig)

    print(f"PDF saved to {out_pdf}")

if __name__ == "__main__":
    main()

