#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : obtain.text.py
#
# Purpose :
#
# Creation Date : 17-12-2015
#
# Last Modified : Thu 17 Dec 2015 01:32:21 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

data = open("/srv/data/huangui2/DataSets/DBLP/paper.dict")

title = dict() 
maxId = 0
for line in data:
  value = line.lower().split("\n")[0].split("\t")
  title[int(value[0])] = value[2] 
  if int(value[0]) > maxId:
    maxId = int(value[0]) + 1 

print maxId

data = open("/srv/data/huangui2/DataSets/DBLP/paper_abstract")
abstract = dict() 

for line in data:
  value = line.lower().split("\n")[0].split("\t")
  abstract[int(value[0])] = value[1] 


fout = open("/srv/data/tensor_embedding/data/title_abs.txt", "w")
for i in xrange(maxId):
  fout.write(title[i])
  if i in abstract:
    fout.write(abstract[i])
  fout.write("\n")
fout.close() 
