import scipy
import sys
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import argparse
import subprocess


def calc_coherence_new(file_dict, min_length, num_subplots, plot_name="mic_coherence.png", show_plot=True):
    if(num_subplots == 1):
        num_rows = 1
        num_cols = 1
    else:
        num_rows = int((num_subplots+1)/2)
        num_cols = 2
    
    non_arr_axes = False
    if num_rows == 1 and num_cols == 1:
        non_arr_axes = True
    elif (num_rows == 1):
        single_dim_axes = True
    else:
        single_dim_axes = False

    fig, axs = plt.subplots(num_rows, num_cols, figsize=(14,7))

    fig.suptitle('Mic coherence plots', fontsize=14)
    row_id = 0
    col_id = 0

    for input_file, ch_pairs in file_dict.items():
        rate, wav_data = scipy.io.wavfile.read(input_file)
        wav_data = wav_data[:min_length]
        max_val = np.iinfo(wav_data.dtype).max
        wav_data = wav_data.astype(dtype=np.float64) / max_val
        wav_data = wav_data.T
        fs = rate
        N = 512.0 * (fs/16000.0)
        overlap = N/2
        for cp in ch_pairs:
            subplot_name = f"{Path(input_file).stem}"
            print(subplot_name)
            f, Cxy = scipy.signal.coherence(wav_data[cp[0]], wav_data[cp[1]], fs, nperseg=N, noverlap=overlap, nfft=N)
            if non_arr_axes:
                axs.set_title(subplot_name)
                axs.set(xlabel='frequency [Hz]', ylabel='Magnitude-Squared Coherence')
                axs.plot(f, Cxy)
            elif single_dim_axes:
                axs[row_id*2 + col_id].set_title(subplot_name)
                axs[row_id*2 + col_id].set(xlabel='frequency [Hz]', ylabel='Magnitude-Squared Coherence')
                axs[row_id*2 + col_id].plot(f, Cxy)
            else:   
                axs[row_id, col_id].set_title(subplot_name)
                axs[row_id, col_id].set(xlabel='frequency [Hz]', ylabel='Magnitude-Squared Coherence')
                axs[row_id, col_id].plot(f, Cxy)

            col_id = col_id + 1
            if col_id == 2:
                col_id = 0
                row_id = row_id + 1

    figinstance = plt.gcf()
    if show_plot:
        plt.tight_layout()
        plt.show()
    figinstance.savefig(plot_name, dpi=200)



def get_args():
    parser = argparse.ArgumentParser("Script to plot coherence between a set of input wav files")
    parser.add_argument("input_files", nargs='*', help="Input .wav files")
    parser.add_argument("--figname", type=str, help="title to add to the output file name", default="mic_coherence.png")
    return parser.parse_args()

if __name__ == "__main__":
    args = get_args()
    
    input_files = []
    for fl in args.input_files:
        rate, wav_data = scipy.io.wavfile.read(fl)
        if rate != 16000: # Downsample to 16kHz
            downsampled_file = f"{Path(fl).stem}_16khz.wav"
            cmd = f"sox {fl} -r 16000 {downsampled_file}"
            subprocess.run(cmd.split(), check=True)
            input_files.append(downsampled_file)
        else:
            input_files.append(fl)

    min_length = None # Minimum length among all files
    for fl in input_files:
        rate, wav_data = scipy.io.wavfile.read(fl)
        if min_length is None:
            min_length = len(wav_data)
        else:
            min_length = min(min_length, len(wav_data))

    print(f"min_length = {min_length}")
    file_dict = {}
    total_num_subplots = 0
    for fl in input_files:
        rate, wav_data = scipy.io.wavfile.read(fl)
        num_channels = wav_data.shape[1]
        combinations = []
        # Create channel combinations
        for i in range(num_channels):
            for j in range(i+1, num_channels):
                combinations.append((i,j))
        file_dict[str(fl)] = combinations
        total_num_subplots = total_num_subplots + len(combinations)

    print("file_dict = ",file_dict)
    print("total_num_subplots = ", total_num_subplots)

    calc_coherence_new(file_dict, min_length, total_num_subplots, plot_name=args.figname)