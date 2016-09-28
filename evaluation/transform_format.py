from math import sqrt
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-emb", help="input path for embedding file")
parser.add_argument("-label", help="input path for label file")
parser.add_argument("-output", help="output path for transformed file")
parser.add_argument("-norm", help="output path for transformed file")
args = parser.parse_args()


node_labels = dict()
with open(args.label, 'r') as label_file:
    for line in label_file:
        elements = line.strip().split('\t')
        node_labels[elements[0].lower()] = elements[1]

in_label = 0
with open(args.output, 'w') as output_file:
    with open(args.emb, 'r') as embedding_file:
        (vocab_size, vector_size) = [int(i) for i in embedding_file.readline().split(' ')]
        for line in embedding_file:
            elements = line.strip().split(' ')
            elements[0] = elements[0].lower()
            count = 1
            if elements[0] in node_labels:
                in_label += 1
                output_file.write(node_labels[elements[0]])
                energy = 0
                for element in elements[1:]:
                    if element.strip() == '':
                        continue
                    # if float(element) < 0.000001:
                    #    element = 0
                    energy += float(element) ** 2
                energy = max(0.000001, sqrt(energy))
                for element in elements[1:]:
                    if element.strip() == '':
                        continue
                    if abs(float(element)) < 0.000001:
                        element = 0
                    if args.norm == '1':
                        output_file.write(' ' + str(count) + ':' + str(float(element) / energy))
                    else:
                        output_file.write(' ' + str(count) + ':' + str(float(element)))
                    count += 1
                output_file.write('\n')
print str(in_label) + "/" + str(len(node_labels)), "discovered in the file"
