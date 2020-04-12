import pandas as pd
import matplotlib.pyplot as plt
import argparse

def parse_args():
    parser = argparse.ArgumentParser(add_help=True, description="Files to plot")
    parser.add_argument("-files", "--files", nargs="+", required=False,
                        default=('cwndVegas.tr', 'cwndNewReno.tr', 'cwndBic.tr'),
                        help="List of files to plot")
    args=parser.parse_args()
    return vars(args)

if __name__ == "__main__":
    args = parse_args()
    for filename in args['files']:
        print(f'Start plotting {filename}')
        filesave = filename.split('.')[0] + '.png'
        df = pd.read_csv(filename, sep=" ", header=None)
        plt.figure()
        plt.plot(df[0], df[1])
        plt.savefig(filesave)
        print(f'End plotting')


