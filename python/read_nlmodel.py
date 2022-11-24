import argparse
import numpy as np
from pathlib import Path
import os

def read_nlm_buffer(nlm_file, gen_set_file):
    buf = np.fromfile(nlm_file, dtype=np.float32)
    rows = int(buf[0])
    cols = int(buf[1])
    print(f"NLModel matrix has {rows} rows and {cols} columns")

    nlm_buf = np.reshape(buf[2:], (rows, cols))
    print(nlm_buf)

    if gen_set_file != None:
        buf[2:].tofile(gen_set_file)


if __name__ == "__main__":
    parser = argparse.ArgumentParser("Read NLModel file")
    parser.add_argument('--nlmodel-file', '-f',
                        help='Binary file containing NLModel buffer read from the device',
                        default=Path( __file__ ).parent / "../nlm_buffer.bin")
    parser.add_argument('--generate-set-file', '-s', help='Generate a .bin file that can be used for testing the set NLModel command', default=None)
    
    args = parser.parse_args()

    if not Path(args.nlmodel_file).is_file():
        print(f"Error: file {args.nlmodel_file} not found")
        exit(1)

    read_nlm_buffer(str(args.nlmodel_file), args.generate_set_file)
