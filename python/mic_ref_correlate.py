import numpy as np
from pathlib import Path
import scipy
import scipy.io.wavfile
import scipy.signal
import argparse
import matplotlib.pyplot as plt
import subprocess

"""
This scripts calculates and plots the correlation lag between the two channels in a stereo wav file.
It is used to check the correlation between the one of the microphone and the reference input to the xvf3800.
It calculates the no. of delay samples for which a peak in the cross correlation between mic and ref is observed.
A positive value of the ref-mic delay indicates a causal filter such that microphones are delayed wrt the reference.
A negative value of the ref-mic delay indicates a non-causal filter such that a signal on the mic is seen earlier than it's seen
on the reference.

Requirements:
    python3
    numpy
    matplotlib
    scipy
    sox

Usage:
    python3 mic_ref_correlate.py <rec_mic0_ref.wav>

The input wav file to the script is expected to be a 2 channel wav file where the first channel
is the recorded mic input and the second channel is the ref input to the device.

Before calculating the correlation, if the sampling freq of the wav file is more than 16KHz, it
is first downsampled to 16KHz using sox. Make sure sox is installed before running this script.
"""
def get_args():
    parser = argparse.ArgumentParser("Script to plot correlation between mic and reference channels")
    parser.add_argument("input_file", type=str, help="Input .wav files")
    return parser.parse_args()

def calc_correlation(data0, data1, rate, plotname):
    N = 32768
    overlap = N/2
    i = 0
    c_delay_full = []
    peak2ave_full = []
    avg_corr = None
    while i+N < min(len(data0), len(data1)):
        start = int(i)
        data0_extract = data0[start:start+N] # mic
        data1_extract = data1[start:start+N] # ref   
        correlation = scipy.signal.correlate(data0_extract, data1_extract, "full")#same pads to get out size = input size
        argmax = np.argmax(np.abs(correlation))
        peak = np.abs(correlation[argmax])
        average = np.mean(np.abs(correlation))
        peak2ave = peak / average
        c_delay = argmax - (N-1)#delay relative to channel 0 (python)
        c_delay_full.append(c_delay)
        peak2ave_full.append(peak2ave)
        if avg_corr is None:
            avg_corr = correlation
        else:
            avg_corr = avg_corr + correlation

        i = i + overlap

    avg_corr = avg_corr / len(c_delay_full)
    t = np.array([i for i in range(len(c_delay_full))], dtype=np.float32)
    t = t * ((N-overlap)/float(rate))

    fig, axes = plt.subplots(3, 1, figsize=(10,8))
    fig.suptitle(f'{Path(plotname).stem}')
    
    axes[0].set_title('ref-mic delay')
    axes[0].set(ylabel='corr delay (samples)', xlabel='time')
    axes[0].plot(t, c_delay_full)
    axes[2].set_title('corr peak2ave ratio')
    axes[2].set(ylabel='peak2ave ratio', xlabel='time')
    axes[2].plot(t, peak2ave_full)
    axes[1].set_title('Mean correlation')
    axes[1].set(ylabel='correlation', xlabel='lag')
    axes[1].plot(np.abs(avg_corr))
    plt.tight_layout()
    figinstance = plt.gcf()
    plt.show()
    print(f"Correlation plot saved in {plotname}")
    figinstance.savefig(plotname)
    return

if __name__ == "__main__":
    args = get_args()

    assert(Path(args.input_file).is_file()), f"Error: {args.input_file} is not a file."
    rate, wav_data = scipy.io.wavfile.read(args.input_file)

    assert rate >= 16000, f"Atleast 16KHz sampling rate input file required. Invalid sampling freq {rate}"
    assert wav_data.ndim == 2 and wav_data.shape[1] == 2, "Only stereo input wav files supported"

    if rate != 16000:
        downsampled_file = f"{Path(args.input_file).stem}_16khz.wav"
        cmd = f"sox {args.input_file} -r 16000 {downsampled_file}"
        subprocess.run(cmd.split(), check=True)
        input_file = downsampled_file
    else:
        input_file = args.input_file

    plotname = "plot_correlation_" + Path(input_file).stem + ".png"
    rate, wav_data = scipy.io.wavfile.read(input_file)
    max_val = np.iinfo(wav_data.dtype).max
    wav_data = wav_data.astype(dtype=np.float64) / max_val
    wav_data = wav_data.T
    data0 = wav_data[0]
    data1 = wav_data[1]
    calc_correlation(data0, data1, rate, plotname)
