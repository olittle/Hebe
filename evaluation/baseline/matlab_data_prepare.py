import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-input", help="input file keeping the events")
parser.add_argument("-output", help="output folder keeping the transformed sparse matrix")
args = parser.parse_args()


networks = list()
used_id = list()

with open(args.input, 'r') as input:
    line = input.readline()
    elements = line.strip().split('\t')
    # excluding weight
    for i in range(len(elements) - 1):
        # dict of edges and node name to id
        networks.append((dict(), dict()))
        used_id.append(0)

with open(args.input, 'r') as input:
    line_count = 0
    for line in input:
        line_count += 1
        elements = line.strip().split('\t')
        for i in range(len(elements[:-1])):
            if i == 1:
                continue
            content = elements[i].strip()
            if len(content) == 0:
                continue
            nodes = content.split('#')
            for node in nodes:
                node_id = 0
                if node in networks[i][1]:
                    node_id = networks[i][1][node]
                else:
                    node_id = used_id[i] + 1
                    used_id[i] += 1
                    networks[i][1][node] = node_id
                edge = (line_count, node_id)
                if edge in networks[i][0]:
                    networks[i][0][edge] += 1
                else:
                    networks[i][0][edge] = 1

for i in range(len(networks)):
    if i == 1:
        continue
    with open(args.output + '/network' + str(i) + '.txt', 'w') as output:
        for (edge, weight) in networks[i][0].iteritems():
            output.write(str(edge[0]))
            output.write('\t')
            output.write(str(edge[1]))
            output.write('\t')
            output.write(str(weight))
            output.write('\n')
    with open(args.output + '/name' + str(i) + '.txt', 'w') as output:
        pool = [None] * used_id[i]
        for (node_name, id) in networks[i][1].iteritems():
            pool[id - 1] = node_name
        for node_name in pool:
            output.write(node_name)
            output.write('\n')
