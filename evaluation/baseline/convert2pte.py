from operator import __add__

import codecs
import sys

def formatter(source_file, dest_path, node_types, center_idx):
	type_names = node_types.split('_')
	types_wo_center = type_names[:].pop(center_idx)
	center_type = type_names[center_idx]
	
	# df = codecs.open(dest_file, 'w+', encoding='utf-8')
	
	nodes = {}
	node_degrees = {i : {} for i in range(len(type_names))}
	
	with codecs.open(source_file, "r", encoding='utf-8') as sf:
		for line in sf:
			groups = line.strip('\r\n').split('\t')
			weight = float(groups[-1])
			center_node = groups[center_idx]
			groups = map(lambda x: x.split('#'), groups[:-1])
			if center_node not in nodes:
				nodes[center_node] = {i : {} for i in range(len(type_names))}
				node_degrees[center_idx][center_node] = 0

			#entities = set()
			for i in range(len(type_names)):
				if i != center_idx:
					for entity in groups[i]:
						if entity == '':
							continue
						#if entity not in entities:
						degree = node_degrees[i].get(entity, 0) + 1
						node_degrees[i][entity] = degree
						node_degrees[center_idx][center_node] += 1
							#entities.add(entity)
						node_weight = nodes[center_node][i].get(entity, 0) + weight
						nodes[center_node][i][entity] = node_weight


	for i in range(len(type_names)):
		type_name = type_names[i]
		df = codecs.open(dest_path + type_name + '.set', 'w+', encoding='utf-8')
		for entity, degree in node_degrees[i].items():
			df.write("%s %d\n" % (entity, degree))
		df.close()

		if i == center_idx:
			continue
		df = codecs.open(dest_path + center_type + '_' + type_name + '.net', 'w+', encoding='utf-8')
		for center_node in nodes:
			for entity, weight in nodes[center_node][i].items():
				df.write("%s %s %f\n" % (center_node, entity, weight)) 
				# df.write("%s %s %f\n" % (entity, center_node, weight))
		df.close()


def formatter_unfied(source_file, dest_path, node_types, center_idx):
	type_name = 'entity'
	
	# df = codecs.open(dest_file, 'w+', encoding='utf-8')
	
	nodes = {}
	node_degrees = {}
	
	with codecs.open(source_file, "r", encoding='utf-8') as sf:
		for line in sf:
			groups = line.strip('\r\n').split('\t')
			weight = float(groups[-1])
			center_node = groups[center_idx]
			event_nodes = reduce(__add__, map(lambda x: x.split('#'), groups[:-1]))
			event_nodes.remove(center_node)

			nodes[center_node] = {}
			node_degrees[center_node] = 0

			for node in event_nodes:
				if len(node) == 0:
					continue
				if node not in nodes:
					nodes[node] = {}
					node_degrees[node] = 0
				nodes[node][center_node] = nodes[node].get(center_node, 0) + weight
				node_degrees[node] = node_degrees[node] + weight
				nodes[center_node][node] = nodes[center_node].get(node, 0) + weight
				node_degrees[center_node] = node_degrees[center_node] + weight


	df = codecs.open(dest_path + type_name + '.set', 'w+', encoding='utf-8')
	for entity, degree in node_degrees.items():
		df.write("%s %d\n" % (entity, degree))
	df.close()

	df = codecs.open(dest_path + type_name + '_' + type_name + '.net', 'w+', encoding='utf-8')
	for node in nodes:
		for entity, weight in nodes[node].items():
			df.write("%s %s %f\n" % (node, entity, weight)) 
			# df.write("%s %s %f\n" % (entity, center_node, weight))
	df.close()


if __name__ == "__main__":
	if len(sys.argv) < 2:
		print "<usage> [source file] [destination path] [node type strings] [center type index] [if heterogeneous]"
		print "<example> python convert2pte.py ../4area/events.txt ./ term_paper_author_venue 1"
		sys.exit(-1)


	# formatter(sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4]))
	if int(sys.argv[5]) > 0:
		formatter(sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4]))
	else:
		formatter_unfied(sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4]))
