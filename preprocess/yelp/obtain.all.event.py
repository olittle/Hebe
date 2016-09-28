#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : obtain.all.event.py
#
# Purpose :
#
# Creation Date : 19-01-2016
#
# Last Modified : Mon 01 Feb 2016 11:11:20 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

import json 
import re 

# first load the business
data = open("/shared/data/tensor_embedding/data/yelp/graph/business.dict") 
busi_name = dict() 
busi_zip = dict() 

for line in data:
  value = line.split("\n")[0].split("\t") 
  business = value[0]
  zipcode = value[-1]
  if len(zipcode) < 2:
    print line 
    
  name = re.sub('[^a-z\-\']+', ' ', value[3].lower())
   
  busi_name[business] = "#".join(name.split()) 
  busi_zip[business] = zipcode 
#  print name, value[3] 
#  print name, zipcode 

print "Load Name Info", "-", len(busi_name) 

# obtain the phrases for each review  
ids_dict = dict() 

count = -1
ids = open("/shared/data/tensor_embedding/data/yelp/raw/reviews_id.all")
for line in ids:
  count += 1 
  value = line.split() 
  ids_dict[count] = value[0]
  
phrase_dict = dict() 
#reviews = open("/shared/data/tensor_embedding/data/yelp/raw/reviews_collection.phrases.clean.frequent")
reviews = open("/shared/data/tensor_embedding/data/yelp/raw/phrases_tf_idf_top_10.txt")
count = -1 
for line in reviews:
  count += 1
  rid = ids_dict[count] 
  value = line.split()
  if len(value) == 0:
    print count + 1
    continue 
  phrase_dict[rid] = "#".join(value) 

print "Load phrases", len(phrase_dict) 

# load review 
data = open("/srv/data/huangui2/DataSets/yelp/yelp_dataset_challenge_academic_dataset/yelp_academic_dataset_review.json")
fout = open("/shared/data/tensor_embedding/data/yelp/event.txt.all.city", "w")
for line in data:
  jdata = json.loads(line) 
  busi = jdata['business_id'] 
  user = jdata['user_id'] 
  rid = jdata['review_id'] 
  
  if busi not in busi_name:
    continue
  
  if rid not in phrase_dict:
    continue
  
  #fout.write("%s\t%s\t%s\t%s\t1.0\n" % (phrase_dict[rid], rid, busi, busi_name[busi]))
  fout.write("%s\t%s\t%s\t%s\t%s\t1.0\n" % (phrase_dict[rid], rid, busi, busi_name[busi], busi_zip[busi]))

fout.close() 
  
