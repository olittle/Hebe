#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : obtain.DBLP.embedding.py
#
# Purpose :
#
# Creation Date : 04-01-2016
#
# Last Modified : Sat 09 Jan 2016 04:11:07 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

import numpy as np 

DataFolder = "/srv/data/huangui2/DataSets/DBLP"

# load embedding from skip-gram 
data = open("/srv/data/tensor_embedding/data/dblp/embedding/embedding_phrase.txt")
embed = dict() 

for line in data:
  value = line.split()
  if len(value) == 2:
    dim = int(value[1])
    continue 
    
  term = value[0]
  embed[term] = np.zeros(dim) 
  for i in range(1, len(value)):
    embed[term][i - 1] = float(value[i]) 

print "term count", len(embed)

target = set() 

#data = open("/srv/data/multi-sense/taxonomy/dblp_graph/topK.txt") 
data = open("/srv/data/multi-sense/taxonomy/dblp_graph/paper_phrases.txt.2") 
pid = -1 
for line in data:
  pid += 1 
  value = line.split("\n")[0].split( ) 
  for v in value:
    target.add(v) 

print "Total number of key phrases", len(target)

fembed = open("/srv/data/tensor_embedding/data/dblp/embedding.txt.full", "w")

fembed.write("%d %d\n" % (len(target), dim))

outvocab = 0
invocab = 0
for phrase in target:
  if len(phrase) == 0:
    continue 
  
  if phrase in embed:
    invocab += 1
    res = embed[phrase]
  else:
    print phrase
    
  vector = [str(res[i]) for i in xrange(dim)]
  fembed.write("%s %s\n" % (phrase, " ".join(vector)))

fembed.close() 

