import argparse
import numpy as np
from pathlib import Path
import matplotlib.pyplot as plt

def read_aec_filter(aec_file, show_plot):
    buf = np.fromfile(aec_file, dtype=np.float32)
    # Convert to 1.31 since that's all I can print from the device
    buf_q31 = np.array(buf*(1<<31), dtype=np.int32)
    #print(buf)
    #print(buf_q31)    
    plt.plot(buf)
    figinstance = plt.gcf()
    if show_plot:
        plt.show()
    figinstance.savefig("aec_filter_demo.png", dpi=200)

if __name__ == "__main__":
    parser = argparse.ArgumentParser("Read AEC filter file")
    parser.add_argument('--aecfilter-file', '-f',
                        help='Binary file containing the AEC filter buffer read from the device',
                        default=Path( __file__ ).parent / "../aec_filter_m0_f0.bin")
    parser.add_argument('--show-plot', '-sp', action='store_true', help='Show the filter plot')
    
    args = parser.parse_args()

    if not Path(args.aecfilter_file).is_file():
        print(f"Error: file {args.aecfilter_file} not found")
        exit(1)

    read_aec_filter(str(args.aecfilter_file), args.show_plot)
