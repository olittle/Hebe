import codecs
from operator import __add__
import sys
import math

def formatter(source_file_1, source_file_2, output_file):
	df = codecs.open(output_file, 'w+', encoding='utf-8')

	node_vec = {}
	meta_line = ''

	with codecs.open(source_file_1, "r", encoding='utf-8') as sf:
		first_line = True
		for line in sf:
			if first_line:
				elements = line.strip('\r\n ').split(' ')
				meta_line = elements[0] + ' ' + str(int(elements[1]) * 2) + '\n'
				first_line = False
				continue
				
			segments = line.strip('\r\n ').split(' ')
			node_name = segments[0]
			if node_name.isdigit():
				continue

			vec_values = map(lambda x: float(x), segments[1:])
			total = math.sqrt(reduce(__add__, map(lambda x : x*x, vec_values)))
			vec_values = map(lambda x: x/total, vec_values)
			node_vec[node_name] = vec_values

	print 'first file read'

	with codecs.open(source_file_2, "r", encoding='utf-8') as sf:
		first_line = True
		for line in sf:
			if first_line:
				first_line = False
				continue
				
			segments = line.strip('\r\n ').split(' ')
			node_name = segments[0]
			if node_name.isdigit():
				continue
				
			vec_values = map(lambda x: float(x), segments[1:])

			total = math.sqrt(reduce(__add__, map(lambda x : x*x, vec_values)))
			vec_values = map(lambda x: x/total, vec_values)
			# print vec_values
			node_vec[node_name].extend(vec_values)

	print 'second file read'

	df.write(meta_line)
	for node in node_vec:
		df.write("%s %s\n" % (node, ' '.join(map(lambda x: str(x), node_vec[node]))))

	df.close()

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print "<usage> [file_1] [file_2] [output]"
		sys.exit(-1)

	formatter(sys.argv[1], sys.argv[2],sys.argv[3])
