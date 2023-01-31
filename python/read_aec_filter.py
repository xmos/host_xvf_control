import argparse
import numpy as np
from pathlib import Path
import matplotlib.pyplot as plt

def read_aec_filter(filelist, show_plot):
    num_files = len(filelist)
    fig, axs = plt.subplots(num_files, 2, figsize=(14,7))
    
    fig.suptitle('AEC impulse response', fontsize=14)
    for i in range(len(filelist)):
        axs[i,0].set_title(f'{filelist[i].name}, time')
        axs[i,1].set_title(f'{filelist[i].name}, freq')
        buf = np.fromfile(str(filelist[i]), dtype=np.float32)
        Buf = np.fft.rfft(buf)
        freq = np.linspace(0, 8000, num=int(len(buf)/2)+1) # Map 0-8kHz into 3072/2 bins
        H = 20*np.log10(np.abs(Buf))
        axs[i,0].plot(buf)
        axs[i,0].set(xlabel='samples', ylabel='Amplitude')
        axs[i,1].plot(freq, H)
        axs[i,1].set(xlabel='frequency(Hz)', ylabel='Magnitude(dB)')
        axs[i,1].set_ylim([max(-75, np.min(H)), np.max(H)+5])
        if i:
            axs[i,0].get_shared_x_axes().join(axs[i,0], axs[0,0])
            axs[i,1].get_shared_x_axes().join(axs[i,1], axs[0,1])
    
    figinstance = plt.gcf()
    if show_plot:
        plt.tight_layout()
        plt.show()

    plot_name = filelist[0].name.split('.')[0] + ".png" 
    figinstance.savefig(plot_name, dpi=200)


if __name__ == "__main__":
    parser = argparse.ArgumentParser("Read AEC filter file")
    parser.add_argument('aecfilter_file', type=str,
                        help='Binary file containing the AEC filter buffer read from the device',
                        )
    parser.add_argument('--num-mics', '-m', type=int, default=4, help='number of mic inputs. Default=4')
    parser.add_argument('--num-ref', '-r', type=int, default=1, help='number of far end inputs. Default=1')
    parser.add_argument('--show-plot', '-sp', action='store_true', help='Show the filter plot')
    
    args = parser.parse_args()
    filelist = []
    for f in range(args.num_ref):
        for m in range(args.num_mics):
            name = Path(str(args.aecfilter_file) + f'.f{f}.m{m}')
            if not name.is_file():
                print(f"Error: file {name} not found")
                exit(1)
            filelist.append(name)

    read_aec_filter(filelist, args.show_plot)
