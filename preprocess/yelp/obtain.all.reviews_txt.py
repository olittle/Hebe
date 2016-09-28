#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : obtain.all.reviews.py
#
# Purpose :
#
# Creation Date : 19-01-2016
#
# Last Modified : Tue 19 Jan 2016 12:11:13 AM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

import json 

data = open("/srv/data/huangui2/DataSets/yelp/yelp_dataset_challenge_academic_dataset/yelp_academic_dataset_review.json")

reviews = open("/srv/data/tensor_embedding/data/yelp/raw/reviews_collection.all", 'w') 
ids = open("/srv/data/tensor_embedding/data/yelp/raw/reviews_id.all", 'w') 

for line in data:
    jdata = json.loads(line)
    reviews.write(jdata["text"].encode('utf-8').replace('\n', ' ').lower() + "\n")
    ids.write(jdata["review_id"] + "\n"); 

reviews.close()
ids.close()

