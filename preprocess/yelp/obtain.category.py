#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : obtain.category.py
#
# Purpose : obtain the top cusines of the restaurants 
#
# Creation Date : 18-01-2016
#
# Last Modified : Mon 18 Jan 2016 11:52:22 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

import json 
import operator 

data = open("/srv/data/huangui2/DataSets/yelp/yelp_dataset_challenge_academic_dataset/yelp_academic_dataset_business.json")

cate_dict = dict()

for line in data:
  jdata = json.loads(line)
  cate = jdata["categories"]
  if jdata["review_count"] < 10:
    continue
  if "Restaurants" not in cate:
    continue
  
  if len(cate) > 2:
    continue 
 
  for x in cate:
    cate_dict[x] = cate_dict.get(x, 0) + 1

sorted_cate = sorted(cate_dict.items(), key=operator.itemgetter(1))
sorted_cate.reverse()

for i in range(30):
  print sorted_cate[i][0], sorted_cate[i][1] 

