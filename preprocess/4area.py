#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : 4area.py
#
# Purpose : generate data in 4 area filtered by conferences 
#
# Creation Date : 10-01-2016
#
# Last Modified : Sun 10 Jan 2016 05:49:20 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.


data = open("/srv/data/tensor_embedding/data/dblp/four_area/conf.txt")
confs = set() 

for line in data:
  value = line.split("\n")[0].split()[1] 
  confs.add(value.lower())
  
print confs 

#data = open("/srv/data/tensor_embedding/data/dblp/events.txt.full") 
data = open("/srv/data/tensor_embedding/data/dblp/events.txt.full.graph") 
fout = open("/srv/data/tensor_embedding/data/dblp/events.txt.4area", "w") 
count = 0 
for line in data:
  value = line.split("\n")[0].split("\t") 
  if len(value) < 5:
    print line 
    exit(1) 
  
  for i in range(len(value)):
    if i == 1 or i == 2 or i == 3:
      count += len(value[i].split("#")) 
    
  value = value[3] 
  if value not in confs:
    continue 
  fout.write(line) 

fout.close()
print count 
