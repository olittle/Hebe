#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : obtain.restaurant.py
#
# Purpose : obtain restaurants 
#
# Creation Date : 18-01-2016
#
# Last Modified : Thu 21 Jan 2016 05:18:33 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

#Mexican 1481
#Chinese 934
#Italian 674
#American (Traditional) 535
#American (New) 495
#Thai 296
#French 242
#Japanese 234
#Vietnamese 187
#Indian 185

import random 
import operator 

data = open("/srv/data/tensor_embedding/data/yelp/graph/business.dict") 

targets = set(["mexican", "pizza", "chinese", "american (traditional)", "italian", "american (new)", "thai", "japanese",  "mediterranean", "indian", "sandwiches"])

cate = dict() 
cate_set = dict() 

for line in data:
  value = line.split("\n")[0].split("\t") 
  rid = value[0] 
  categories = value[1].lower().split("|") 
  cid = int(value[2]) 

  if cid < 30:
    continue 
    
#  if len(categories) != 2:
#    continue 

  hit = set() 
  
  for c in categories:
    if c == "restaurants":
      continue
    if c in targets:
      hit.add(c) 
    
  if len(hit) != 1:
    continue
    
  for c in hit:
    cate[c] = cate.get(c, 0) + 1
    if c not in cate_set:
      cate_set[c] = set()
    cate_set[c].add(rid) 
 
sorted_x = sorted(cate.items(), key=operator.itemgetter(1))
sorted_x.reverse() 

for i in xrange(len(sorted_x)):
  print sorted_x[i][0], sorted_x[i][1]

cate_cnt = dict() 
cnt = 0
for c in cate_set:
  cnt += 1
  cate_cnt[c] = cnt  

fout = open("/srv/data/tensor_embedding/data/yelp/business.label", 'w')
for c in cate_set:
  res = random.sample(cate_set[c], 100) 
  for i in res:
    fout.write("%s\t%d\n" % (i, cate_cnt[c])) 
fout.close() 

