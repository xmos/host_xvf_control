import argparse
import numpy as np
from pathlib import Path
import os

def read_nlm_buffer(nlm_file, gen_set_file, rows, cols):
    buf = np.fromfile(nlm_file, dtype=np.float32)
    print(f"NLModel matrix has {rows} rows and {cols} columns")

    nlm_buf = np.reshape(buf, (rows, cols))
    print(nlm_buf)

    if gen_set_file != None:
        buf.tofile(gen_set_file)


if __name__ == "__main__":
    parser = argparse.ArgumentParser("Read NLModel file")
    parser.add_argument('--nlmodel-file', '-f',
                        help='Binary file containing NLModel buffer read from the device',
                        default=Path( __file__ ).parent / "../nlm_buffer.bin.r16.c40")
    parser.add_argument('--generate-set-file', '-s', help='Generate a .bin file that can be used for testing the set NLModel command', default=None)
    parser.add_argument('--Ncols', help='Number of coulums in NLModel', default=40)
    parser.add_argument('--Nrows', help='Number of rows in NLModel', default=16)
    
    args = parser.parse_args()

    if not Path(args.nlmodel_file).is_file():
        print(f"Error: file {args.nlmodel_file} not found")
        exit(1)

    read_nlm_buffer(str(args.nlmodel_file), args.generate_set_file, args.Ncols, args.Nrows)
