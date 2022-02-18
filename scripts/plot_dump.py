"""different plots of dumpfiles
"""
import sys
import matplotlib.pyplot as plt
import read_dump

def plot_boxplot(filename, output):
    """plot boxplots out of dump file

    Args:
        filename (str): dump file to plot
        output (str): where to write the figure (filename.pdf)
    """
    list_dict = read_dump.read_dump(filename)[1]
    fig1 = plt.figure()
    fig1.add_axes([0,0,1,1])
    times = [item[1] for item in list_dict.items()]
    plt.boxplot(times, vert=True, patch_artist=True, labels=list_dict.keys())
    #plt.show()
    plt.ylabel('Execution time in ms')
    plt.savefig(output, bbox_inches='tight')


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Please provide the name to the dumpfile and the name of the figure file (.pdf).")
    else:
        dumpfile = sys.argv[1]
        outfile = sys.argv[2]
        plot_boxplot(dumpfile, outfile)
