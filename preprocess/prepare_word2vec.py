#-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#
# File Name : prepare_word2vec.py
#
# Purpose : prepare dataset for word2vec
#
# Creation Date : 22-01-2016
#
# Last Modified : Fri 22 Jan 2016 11:59:42 PM CST
#
# Created By : Huan Gui (huangui2@illinois.edu) 
#
#_._._._._._._._._._._._._._._._._._._._._.

import sys
import os
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-input", help="input path for events folder")
args = parser.parse_args()


def generate_link():
  # Build dictionaries
  term_term = dict()
  eng_stopwords = set()
  with open('preprocess/stopwords.txt') as input:
    for line in input:
      eng_stopwords.add(line.strip())

  data = open(args.input + "/events.txt")
  cnt = -1
  for line in data:
    cnt += 1
    value = line.split("\t")[0].split("#")
    terms = value 
#    vLen = len(value) 
#    for vi in xrange(vLen):
#      v = value[vi] 
#      if v not in eng_stopwords and len(v) > 1:
#        terms.append(v)

    tCnt = len(terms)
    iWindows = 10
    windows = iWindows * 1.0

    for i in xrange(tCnt):
      t = terms[i]
      for j in xrange(1, iWindows + 1):
        if i + j >= tCnt:
          break
        ct = terms[i + j]

        if ct > t:
          word = "%s|%s" % (t, ct)
        else:
          word = "%s|%s" % (ct, t)

        co_weight = (windows - j + 1) / windows
        term_term[word] = term_term.get(word, 0) + co_weight

  print "start writing.."
  T_T = open(args.input + "/words_edges.txt", "w")

  for pair in term_term:
    words = pair.split("|")
    T_T.write("%s\t%s\t%f\n" % (words[0], words[1],term_term[pair]))
    # T_T.write("%s\t%s\t%f\n" % (words[1], words[0],term_term[pair]))

  T_T.close()

  return

if __name__ == "__main__":
  generate_link()
  
