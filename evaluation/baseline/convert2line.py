from operator import __add__
import codecs
import sys

def formatter(source_file, dest_file, center_idx):
	df = codecs.open(dest_file, 'w+', encoding='utf-8')
	nodes = {}
	
	with codecs.open(source_file, "r", encoding='utf-8') as sf:
		for line in sf:
			groups = line.split('\n')[0].split('\t')
			weight = float(groups[-1])
			center_node = groups[center_idx]
		 
			if center_node in nodes:
				print "Duplicated paper", center_node
				exit(1) 
				
			nodes[center_node] = {} 
			
			for i in xrange(len(groups)):
				if i == center_idx: 
					continue
				value = groups[i].split('#')
			 
				for node in value:
					nodes[center_node][node] = nodes[center_node].get(node, 0) + weight

	for node in nodes:
		for node_2 in nodes[node]:
			df.write("%s\t%s\t%f\n" % (node, node_2, nodes[node][node_2]))
			df.write("%s\t%s\t%f\n" % (node_2, node, nodes[node][node_2]))
	df.close()


def formatter_no_term(source_file, dest_file, center_idx):
	df = codecs.open(dest_file, 'w+', encoding='utf-8')
	nodes = {}
	
	with codecs.open(source_file, "r", encoding='utf-8') as sf:
		for line in sf:
			groups = line.split('\n')[0].split('\t')
			weight = float(groups[-1])
			center_node = groups[center_idx]
		 
			if center_node in nodes:
				print "Duplicated paper", center_node
				exit(1) 
				
			nodes[center_node] = {} 
			
			for i in xrange(1, len(groups) - 1):
				if i == center_idx: 
					continue
				value = groups[i].split('#')  
			 
				for node in value:
					nodes[center_node][node] = nodes[center_node].get(node, 0) + weight

	for node in nodes:
		for node_2 in nodes[node]:
			df.write("%s\t%s\t%f\n" % (node, node_2, nodes[node][node_2]))
			df.write("%s\t%s\t%f\n" % (node_2, node, nodes[node][node_2]))
	df.close()
					

# The formatter without paper as center
def formatter_decentralized(source_file, dest_file, center_idx):
	df = codecs.open(dest_file, 'w+', encoding='utf-8')
	
	nodes = {}
	
	with codecs.open(source_file, "r", encoding='utf-8') as sf:
		for line in sf:
			groups = line.strip('\r\n').split('\t')
			weight = float(groups[-1])
			groups = map(lambda x: x.split('#'), groups[:-1])
			groups.pop(center_idx)

			for i in range(len(groups)):
				group = groups[i]
				for node in group:
					if node not in nodes:
						nodes[node] = {}
					for j in range(i + 1, len(groups)):
						group_2 = groups[j]
						for node_2 in group_2:
							nodes[node][node_2] = nodes[node].get(node_2, 0) + weight

		for node in nodes:
			for node_2 in nodes[node]:
				df.write("%s\t%s\t%f\n" % (node, node_2, nodes[node][node_2]))
				df.write("%s\t%s\t%f\n" % (node_2, node, nodes[node][node_2]))
	df.close()



if __name__ == "__main__":
	if len(sys.argv) < 3:
		print "<usage> [source file] [destination file] [center type index]"
		sys.exit(-1)

	# formatter(sys.argv[1], sys.argv[2], int(sys.argv[3]))
	# formatter_decentralized(sys.argv[1], sys.argv[2], int(sys.argv[3]))
	formatter_no_term(sys.argv[1], sys.argv[2], int(sys.argv[3]))

