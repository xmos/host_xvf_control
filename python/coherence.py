import scipy
import sys
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import argparse
import subprocess

def calc_coherence(all_data, file_ch_tags, rate, plot_name="mic_coherence.png", show_plot=True):
    num_inputs = len(all_data)
    print(num_inputs)
    combinations = []
    for i in range(num_inputs):
        for j in range(i+1, num_inputs):
            combinations.append((i,j))

    print(f"combinations={combinations}, {len(combinations)}")
    if len(combinations) > 1:
        assert((len(combinations) % 2) == 0), "Don't have time to figure out plotting odd number of combinations"

    if(len(combinations) == 1):
        num_rows = 1
        num_cols = 1
    else:
        num_rows = int(len(combinations)/2)
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
    fs = rate
    N = 512.0 * (fs/16000.0)
    overlap = N/2

    for comb in combinations:
        ch0 = comb[0]
        ch1 = comb[1]
        data0 = all_data[ch0]
        data1 = all_data[ch1]
        f, Cxy = scipy.signal.coherence(data0, data1, fs, nperseg=N, noverlap=overlap, nfft=N)

        subplot_name = f'{file_ch_tags[ch0]}_WITH_{file_ch_tags[ch1]}'
        print(subplot_name)
        
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
    return

def get_args():
    parser = argparse.ArgumentParser("Script to plot coherence between a set of input wav files")
    parser.add_argument("input_files", nargs='*', help="Input .wav files")
    parser.add_argument("--ch-index", type=str,
                        help="<file index>_<index of the channel used in coherence calculation>.")
    parser.add_argument("--figname", type=str, help="title to add to the output file name")
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

    file_ch_list = []
    if(args.ch_index != None):
        lst = args.ch_index.split() # Split into list of "file_column" values
        for f in lst:
            file_ch_list.append((int(f.split('_')[0]), int(f.split('_')[1])))
    print(f"file_ch_list = {file_ch_list}")
    # Collect all the channels that we're interersted in calculating the coherence for
    file_ch_tags = []
    all_data = None
    for (fl, ch) in file_ch_list:
        rate, wav_data = scipy.io.wavfile.read(input_files[fl])
        wav_data = wav_data[:min_length]
        max_val = np.iinfo(wav_data.dtype).max
        wav_data = wav_data.astype(dtype=np.float64) / max_val
        wav_data = wav_data.T
        if all_data is None:
            all_data = wav_data[ch]
        else:
            all_data = np.vstack([all_data, wav_data[ch]])
        print("all_data.shape = ", all_data.shape)
        tag = Path(input_files[fl]).stem + f"_ch{ch}"
        file_ch_tags.append(tag)

    print("file_ch_tags = ", file_ch_tags)

    calc_coherence(all_data, file_ch_tags, rate)