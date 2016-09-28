#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : convert-2_.py
#
# Purpose : convert- 2 _ 
#
# Creation Date : 17-01-2016
#
# Last Modified : Wed 20 Jan 2016 09:03:00 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

import re 

data = open("/srv/data/multi-sense/taxonomy/dblp_graph/paper_phrases.txt.1")
fout = open("/srv/data/multi-sense/taxonomy/dblp_graph/paper_phrases.txt.clean", "w")

#data = open("/srv/data/tensor_embedding/data/yelp/raw/reviews_collection.phrases.clean")
#fout = open("/srv/data/tensor_embedding/data/yelp/raw/reviews_collection.phrases.clean.2", "w")
count = 0
for line in data:
  
  count += 1
  if len(line) == 1: 
    print "empty", count
    fout.write(line) 
    continue
  
  Flag = False 
  while not ((line[0] >= 'a' and line[0] <= 'z')):
    line = line[1:]
    if len(line) == 1:
      print "weird", count
      fout.write(line) 
      Flag = True 
      break 
  
  if Flag:
    continue 
  
  line = re.sub('[-_ ][-_ ]+', " ", line) 
  while re.match('[-_ ][-_ ]+', line) != None:
    print "test" 
    line = re.sub('[-_ ][-_ ]+', " ", line) 

#  while line.find("--") != -1:
#    line = line.replace("--", " ")
#  while line.find("- ") != -1:
#    line = line.replace("- ", " ")
#  while line.find(" -") != -1:
#    line = line.replace(" -", " ")
#  while line.find(" - ") != -1:
#    line = line.replace(" - ", " ")
#  while line.find("-\n") != -1:
#    line = line.replace("-\n", "\n")
#    
#  while line.find("__") != -1:
#    line = line.replace("__", " ")
#  while line.find("_ ") != -1:
#    line = line.replace("_ ", " ")
#  while line.find(" _") != -1:
#    line = line.replace(" _", " ")
#  while line.find(" _ ") != -1:
#    line = line.replace(" _ ", " ")
#  while line.find("_\n") != -1:
#    line = line.replace("_\n", "\n")
 
  line = line.replace("-", "_")
  while line.find("_\n") != -1:
    line = line.replace("_\n", "\n")
  
  fout.write(line)
  if count % 100000 == 0:
    print count 

fout.close() 
