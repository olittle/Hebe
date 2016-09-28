#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : obtain.restaurant.py
#
# Purpose : obtain restaurants 
#
# Creation Date : 18-01-2016
#
# Last Modified : Wed 10 Feb 2016 03:56:47 PM CST
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

import json
import random 

data = open("/shared/data/tensor_embedding/data/yelp/raw/yelp_dataset_challenge_academic_dataset/yelp_academic_dataset_business.json")

state_cnt = dict()

cuisine_cnt = dict() 

cuisine_set = dict() 

target = ['mexican', 'chinese', 'italian', 'american (traditional)', 'american (new)', 'thai', 'french', 'japanese', 'vietnamese', 'indian'] 

for t in target:
  cuisine_set[t] = []

target = set(target) 

for line in data:
  jdata = json.loads(line)

  cate = jdata["categories"]
  if "Restaurants" not in cate:
    continue
  
  if len(cate) != 2: 
    continue
  
  if jdata['review_count'] < 50:
    continue 
 
  zipCode = jdata['full_address'].replace('\n', ' ').split("\n")[0].split()[-1]
  if len(zipCode) != 5:
    continue

  Flag = True 
  for c in cate:
    x = c.lower()
    if x == 'restaurants':
      continue 
    if x not in target:
      Flag = False 
      break
    cuisine_cnt[x] = cuisine_cnt.get(x, 0) + 1 
    cuisine_set[x].append(jdata["business_id"]) 

  if Flag == False:
    continue 
    
fout = open("/srv/data/tensor_embedding/data/yelp/business.label", 'w')
for x in cuisine_set:
  res = random.sample(cuisine_set[x], 100) 
  for i in res:
    fout.write("%s\t%s\n" % (i, x)) 
fout.close()

for x in cuisine_cnt:
  print x, cuisine_cnt[x]
