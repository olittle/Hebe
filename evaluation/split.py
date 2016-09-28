import argparse
import random
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("-input", help="input file keeping the data")
parser.add_argument("-ratio", help="split ratio for training data")
args = parser.parse_args()


def file_len(fname):
    p = subprocess.Popen(['wc', '-l', fname], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    result, err = p.communicate()
    if p.returncode != 0:
        raise IOError(err)
    return int(result.strip().split()[0])


training_ratio = float(args.ratio)
length = file_len(args.input)
random.seed(100)
indicators = [random.random() < training_ratio for i in xrange(length)]
with open(args.input + '.train', 'w') as output1:
    with open(args.input + '.test', 'w') as output2:
        with open(args.input, 'r') as input:
            j = 0
            for line in input:
                if indicators[j]:
                    output1.write(line.strip())
                    output1.write('\n')
                else:
                    output2.write(line.strip())
                    output2.write('\n')
                j += 1
