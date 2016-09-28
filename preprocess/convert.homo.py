#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : convert.homo.py
#
# Purpose : convert the heterogeneous information network into homogeneous information network 
#
# Creation Date : 09-01-2016
#
# Last Modified : Sat 09 Jan 2016 10:44:19 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

data = open("/srv/data/tensor_embedding/data/dblp/events.txt.full")
fout = open("/srv/data/tensor_embedding/data/dblp/events.homo", "w")

for line in data:
  value = line.split("\n")[0].split("\t")
  terms = value[0].split("#")
  paper = value[1]
  author = value[2].split("#")
  venue = value[3] 
  
  term_count = dict() 
  for t in terms:
    term_count[t] = term_count.get(t, 0) + 1.0 
  
  for t in term_count:
    fout.write("%s %s %f\n" % (t, paper, term_count[t]))
    fout.write("%s %s %f\n" % (paper, t, term_count[t]))
  
  for a in author:
    fout.write("%s %s 1.0 \n" % (a, paper))
    fout.write("%s %s 1.0 \n" % (paper, a))
    
  fout.write("%s %s 1.0\n" % (venue, paper)) 
  fout.write("%s %s 1.0\n" % (paper, venue)) 

fout.close() 


