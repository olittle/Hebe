#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : remove_infrequent_words.py
#
# Purpose : remove infrequent words from the phrase data
#
# Creation Date : 05-01-2016
#
# Last Modified : Sat 09 Jan 2016 04:09:39 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

import numpy as np 

from nltk.corpus import stopwords
stopwordset = set(stopwords.words("english"))

data = open("/srv/data/tensor_embedding/data/dblp/embedding/embedding_phrase.txt")
embed = dict() 

for line in data:
  value = line.split()
  if len(value) == 2:
    dim = int(value[1])
    continue
  
  term = value[0]
  if term in stopwordset:
    continue 
  
  embed[term] = np.zeros(dim) 
  for i in range(1, len(value)):
    embed[term][i - 1] = float(value[i])

print len(embed) 

input_file = "/srv/data/multi-sense/taxonomy/dblp_graph/paper_phrases.txt"
out_file = "/srv/data/multi-sense/taxonomy/dblp_graph/paper_phrases.txt.2"

data = open(input_file)
fout = open(out_file, "w") 

count = 0

for line in data:
  value = line.split() 
  out = []
  for v in value:
    if v in embed:
      out.append(v)
  if len(out) == 0:
    pass
  else:
    count += 1 
  fout.write("%s\n" % " ".join(out)) 

fout.close()
print count 

