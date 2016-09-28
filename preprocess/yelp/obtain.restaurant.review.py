#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : obtain.restaurant.review.py
#
# Purpose : obtain the review text for the selected restaurants 
#
# Creation Date : 18-01-2016
#
# Last Modified : Mon 18 Jan 2016 11:22:52 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

import json 

lables = open("/srv/data/tensor_embedding/data/yelp/business.label")
restaurants = [] 

for line in lables:
  value = line.split() 
  restaurants.append(value[0]) 
restaurants = set(restaurants) 

data = open("/srv/data/huangui2/DataSets/yelp/yelp_dataset_challenge_academic_dataset/yelp_academic_dataset_review.json")

reviews = open("/srv/data/tensor_embedding/data/yelp/raw/reviews_collection", 'w') 
ids = open("/srv/data/tensor_embedding/data/yelp/raw/reviews_id", 'w') 

for line in data:
    jdata = json.loads(line)
    business = jdata["business_id"] 
    if business not in restaurants:
      continue 
    reviews.write(jdata["text"].encode('utf-8').replace('\n', ' ').lower() + "\n")
    ids.write(jdata["review_id"] + "\n"); 

reviews.close()
ids.close()
