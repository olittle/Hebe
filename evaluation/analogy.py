from math import sqrt
from numpy import dot, linalg, shape, zeros
from os import walk
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-input", help="input folder for embedding file")
args = parser.parse_args()


types = []
embeddings = dict()
node_names = dict()
for (dirpath, dirnames, filenames) in walk(args.input):
    for filename in filenames:
        if not filename.endswith("embedding.txt"):
            continue
        with open(dirpath + filename, 'r') as embedding_file:
            types.append(filename[:filename.find('_')])
            print "Loading", types[-1], "from", filename
            node_names[types[-1]] = dict()
            (vocab_size, vector_size) = [int(i) for i in embedding_file.readline().split(' ')]
            embeddings[types[-1]] = zeros((vocab_size, vector_size), dtype=float)
            node_id = 0
            content = ""
            for line in embedding_file:
                elements = line.strip().split(' ')
                node_name = elements[0].lower().strip()
                node_names[types[-1]][node_id] = node_name
                node_names[types[-1]][node_name] = node_id
                embeddings[types[-1]][node_id] = list(map(float, elements[1:]))
                node_id += 1
            row_sums = linalg.norm(embeddings[types[-1]], axis=1)
            # embeddings[types[-1]] = (embeddings[types[-1]].T / row_sums).T
if "term" in embeddings and "context" in embeddings:
    print "Merge term embeddings with context"
    embeddings["term"] += embeddings["context"]
    embeddings["term"] /= 2

# norm_embeddings = dict()
for (key, val) in embeddings.iteritems():
    row_sums = linalg.norm(val, axis=1)
    embeddings[key] = (val.T / row_sums).T

while True:
    line = raw_input("Enter formula (type:node_name, support + and -): ")
    embedding = zeros((1, vector_size), dtype=float)
    token = ''
    last_op = '+'
    type_count = dict()
    error = False
    line += '\n'
    total_count = 0
    for ch in line:
        if ch == '+' or ch == '-' or ch == '\n':
            token = token.strip().lower()
            elements = token.split(':')
            if len(elements) < 2:
                error = True
                break
            node_type = elements[0].strip()
            node_name = elements[1].strip()
            if node_type not in node_names:
                print "Node type", node_type, "not exists!"
                error = True
                break
            if node_name not in node_names[node_type]:
                print "Node", node_name, "not exists!"
                error = True
                break
            if last_op == '+':
                embedding += embeddings[node_type][node_names[node_type][node_name]]
                # print str(sqrt(sum(embeddings[node_type][node_names[node_type][node_name]]**2)))
            else:
                embedding -= embeddings[node_type][node_names[node_type][node_name]]
                # print str(sqrt(sum(embeddings[node_type][node_names[node_type][node_name]]**2)))
            token = ''
            if node_type in type_count:
                type_count[node_type] += 1
            else:
                type_count[node_type] = 1
            total_count += 1
            last_op = ch
        else:
            token += ch
    if error:
        continue
    # get target type
    target_type = types[0]
    if total_count == 1:
        target_type = "term"
    else:
        for node_type, count in type_count.iteritems():
            if count == 1:
                target_type = node_type
                break
    print 'Target type:', target_type
    sim = dot(embedding, embeddings[target_type].T) / sqrt(sum(sum(embedding**2)))
    node_sim = sim.tolist()[0]
    sim_node_indices = sorted(range(len(node_sim)), key=lambda k: -node_sim[k])
    print '%50s %20s' % ('Word', 'Cosine distance')
    print '-----------------------------------------------------------------------'
    for i in sim_node_indices[:30]:
        print '%50s %20s' % (node_names[target_type][i], node_sim[i])

# venue:kdd - term:data_mining + term:databases
# author:jiawei_han - venue:kdd + venue:nips
# venue:kdd - term:data_mining + term:computer_vision
# term:data_mining + venue:icml - venue:kdd
# term:data_mining + venue:acl - venue:kdd
# term:machine_learning - venue:icml + venue:kdd
# term:frequent_patterns - author:jiawei_han + author:michael_i._jordan
# term:frequent_patterns - author:jiawei_han + author:geoffrey_e._hinton
# venue:nips - term:machine_learning + term:data_mining
