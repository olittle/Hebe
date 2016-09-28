#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : construct_doc_phrase_graph.py
#
# Purpose : Construct the doc phrase graph to learn the embedding of the graph  
#
# Creation Date : 05-01-2016
#
# Last Modified : Tue 05 Jan 2016 04:53:29 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

# obtain the words having embedding
data = open("/srv/data/tensor_embedding/data/dblp/embedding/embedding_phrase.txt")
vocab = set() 
for line in data:
  value = line.split() 
  if len(value) < 4:
    continue 
  vocab.add(value[0]) 

data = open("/srv/data/multi-sense/taxonomy/dblp_graph/paper_phrases.txt")
#fout = open("/srv/data/tensor_embedding/data/dblp/doc_phrases", "w")
fout2 = open("/srv/data/tensor_embedding/data/dblp/phrase_doc.txt", "w")
pid = -1 
for line in data:
  pid += 1 
  phrases = dict() 
  
  value = line.split() 
  for v in value:
    if v not in vocab:
      continue 
    phrases[v] = phrases.get(v, 0) + 1 

  for v in phrases:
    #fout.write("%d %s %d\n" % (pid, v, phrases[v]))
    fout2.write("%s %d %d\n" % (v, pid, phrases[v]))
fout2.close() 

