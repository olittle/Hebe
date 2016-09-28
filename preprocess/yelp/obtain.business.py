#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : obtain.business.py
#
# Purpose : obtain the dictionary for the restaurant  
#
# Creation Date : 19-01-2016
#
# Last Modified : Tue 19 Jan 2016 03:26:21 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

import json

data = open("/srv/data/huangui2/DataSets/yelp/yelp_dataset_challenge_academic_dataset/yelp_academic_dataset_business.json")
fout = open("/srv/data/tensor_embedding/data/yelp/graph/business.dict", 'w')
state_cnt = dict()

for line in data:
  jdata = json.loads(line)

  cate= jdata["categories"]
  if "Restaurants" not in cate:
    continue

  zipCode = jdata['full_address'].replace('\n', ' ').split("\n")[0].split()[-1]
  if len(zipCode) != 5:
    continue
  
  if jdata["review_count"] < 10:
    continue

  try:
    fout.write(jdata["business_id"].encode('ascii', 'ignore') + "\t" + "|".join(jdata["categories"]) + "\t" + str(jdata["review_count"]) + "\t" + jdata["name"].encode('ascii', 'ignore') + "\t" + jdata["city"].encode('ascii', 'ignore') + "\t" + jdata["state"].encode('ascii', 'ignore') + "\t" + zipCode.encode('ascii', 'ignore') + "\n")
  except:
    print jdata["business_id"].encode('ascii', 'ignore') 
    print "|".join(jdata["categories"]) 
    print str(jdata["latitude"]) 
    print str(jdata["longitude"]) 
    print jdata["name"].encode('ascii', 'ignore') 
    print jdata["full_address"].encode('ascii', 'ignore').replace('\n', ' ') 
    print "zip", zipCode 
    print jdata["state"].encode('ascii', 'ignore') 

  state = jdata["state"]

  state_cnt[state] = state_cnt.get(state, 0) + 1

fout.close()

for st in state_cnt:
  print st, state_cnt[st]

