#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : obtain.event.py
#
# Purpose : Create the hyper edge of the events 
#
# Creation Date : 04-01-2016
#
# Last Modified : Tue 02 Feb 2016 12:02:00 AM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

import unicodedata

def remove_accents(input_str):
  input_str = unicode(input_str, "utf-8")
  nfkd_form = unicodedata.normalize('NFKD', input_str)
  only_ascii = nfkd_form.encode('ASCII', 'ignore')
  return only_ascii

tbk = set() 
data = open("/shared/data/tensor_embedding/data/dblp/frequent/label-area.txt")
for line in data:
  value = line.split()[0].lower()
  value = remove_accents(value) 
  tbk.add(value)

data = open("/shared/data/tensor_embedding/data/dblp/frequent/label-group.txt")
for line in data:
  value = line.split()[0].lower()
  value = remove_accents(value) 
  tbk.add(value)
  
# Schema:
# Paper_Id, Author_set, Venue_Id, Year, Phrase_set

freq_threshold = 5 

DataFolder = "/srv/data/huangui2/DataSets/DBLP"
# Load the frequency of authors, venues
freq_author = dict() 
data = open("%s/paper_author" % DataFolder)
for line in data:
  value = line.split()
  aid = int(value[1]) 
  freq_author[aid] = freq_author.get(aid, 0) + 1
  
# Load the frequency of venues 
freq_venue = dict() 
data = open("%s/paper_venue" % DataFolder) 
for line in data:
  value = line.split()
  vid = int(value[1]) 
  freq_venue[vid] = freq_venue.get(vid, 0) + 1
  
# Load Author 
author_dict = dict() 
data = open("%s/author.dict" % DataFolder)
for line in data:
  value = line.split("\n")[0].split("\t")
  if len(value[1]) < 2:
    continue 
  aid = int(value[0])
  name = remove_accents(value[1]) 
  name = name.lower().replace(" ", "_")
  if freq_author[aid] <= freq_threshold and name not in tbk:
    continue 
  author_dict[aid] = name 

print "Load Author Dict"

# Load Venue 
venue_dict = dict() 
data = open("%s/venue.dict" % DataFolder)
for line in data:
  value = line.split("\n")[0].split("\t") 
  vid = int(value[0])
  if len(value[2]) < 2:
    continue
  if freq_venue[vid] <= freq_threshold:
    continue
  name = remove_accents(value[2]) 
  name = name.lower().replace(" ", "_")
  venue_dict[vid] = name

print "Load Venue Dict"

# Load Paper-Author 
Paper_Author = dict()
data = open("%s/paper_author" % DataFolder) 
for line in data:
  value = line.split() 
  pid = int(value[0]) 
  aid = int(value[1])
  if pid not in Paper_Author:
    Paper_Author[pid] = [] 
  if aid in author_dict:
    Paper_Author[pid].append(author_dict[aid]) 

print "Load Paper_Author"

# Load Paper-Venue 
Paper_Venue = dict() 
data = open("%s/paper_venue" % DataFolder) 
for line in data:
  value = line.split() 
  pid = int(value[0]) 
  vid = int(value[1])
  if vid not in venue_dict:
    continue 
  if pid in Paper_Venue:
    print "Error %d in two venues??" % pid 
  Paper_Venue[pid] = venue_dict[vid]  

print "Load Paper_Venue"

# Load Paper-Phrases
Paper_Phrases = dict()
vocab = set() 
#data = open("%s/paper_phrases" % DataFolder) 
#data = open("/srv/data/multi-sense/taxonomy/dblp_graph/topK.txt") 
#data = open("/srv/data/multi-sense/taxonomy/dblp_graph/paper_phrases.txt.2") 
#data = open("/shared/data/tensor_embedding/data/dblp/raw/phrases_tf_idf_top_10.txt") 
data = open("/shared/data/tensor_embedding/data/dblp/raw/paper_phrases.txt.frequent") 
pid = -1 
for line in data:
  pid += 1 
  value = line.split("\n")[0].split() 
  Paper_Phrases[pid] = []
  for v in value:
    Paper_Phrases[pid].append(v)
    vocab.add(v) 

print "Total vocab", len(vocab) 
print "Load Paper_Phrases"

# Load Paper Info
fevent = open("/shared/data/tensor_embedding/data/dblp/frequent/events.txt", "w")
data = open("%s/paper.dict" % DataFolder) 

for line in data:
  value = line.split() 
  pid = int(value[0])
  year = value[1]
  if len(year) < 2: continue
  if len(Paper_Author[pid]) == 0 or pid not in Paper_Venue:
    continue 
  if len(Paper_Phrases[pid]) > 0:
    fevent.write("%s\t%d\t%s\t%s\t1.0\n" % ("#".join(Paper_Phrases[pid]), pid, "#".join(Paper_Author[pid]), Paper_Venue[pid]))
#    fevent.write("%s\t%d\t%s\t%s\t%s\t1.0\n" % ("#".join(Paper_Phrases[pid]), pid, "#".join(Paper_Author[pid]), Paper_Venue[pid], year))

fevent.close() 

