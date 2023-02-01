import numpy as np
from pathlib import Path
import scipy
import argparse
import matplotlib.pyplot as plt
import subprocess

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
    figinstance.savefig(plotname)
    return

if __name__ == "__main__":
    args = get_args()

    assert(Path(args.input_file).is_file()), f"Error: {args.input_file} is not a file."
    rate, wav_data = scipy.io.wavfile.read(args.input_file)
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
    assert(len(wav_data) == 2), f"Error: Only stereo files supported. {len(wav_data)} channels file given."
    data0 = wav_data[0]
    data1 = wav_data[1]
    calc_correlation(data0, data1, rate, plotname)
