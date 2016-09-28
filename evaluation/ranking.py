import argparse
import numpy as np

parser = argparse.ArgumentParser()
parser.add_argument("-input", help="input path for file")
args = parser.parse_args()

node_emb = []
group = dict()
with open(args.input, 'r') as embedding_file:
    count = 0
    for line in embedding_file:
        elements = line.strip().split(' ')
        elements[0] = elements[0].lower()
        node_emb.append([float(a.split(':')[1]) for a in elements[1:]])
        group[count] = elements[0]
        count += 1

embedding = np.matrix(node_emb)

# row_sums = np.linalg.norm(embedding, axis=1)
# embedding = (embedding.T / row_sums).T
sim = embedding * embedding.T


total_auc = 0
for index in range(len(node_emb)):
    node_sim = sim[index].tolist()[0]
    sim_indices = sorted(range(len(node_sim)), key=lambda k: -node_sim[k])
    label = group[index]
    num_rel_items = 0
    for i in sim_indices:
        if group[i] == label:
            num_rel_items += 1

    num_eval_items = len(sim_indices)
    num_eval_pairs = (num_eval_items - num_rel_items) * num_rel_items
    auc = 0
    if num_eval_pairs == 0:
        auc = 0.5
    else:
        num_correct_pairs = 0
        hit_count = 0
        for i in sim_indices:
            if group[i] != label:
                num_correct_pairs += hit_count
            else:
                hit_count += 1
        auc = float(num_correct_pairs) / num_eval_pairs
    total_auc += auc

print 'AUC:', total_auc / len(node_emb)
