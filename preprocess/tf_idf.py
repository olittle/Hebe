from sklearn.feature_extraction.text import TfidfVectorizer

input_file = "/srv/data/multi-sense/taxonomy/dblp_graph/paper_phrases.txt.2"
output_file = "topK.txt"

top_K = 5

corpus = []
count = 10000
with open(input_file, 'r') as input:
    for line in input:
        corpus.append(line.split("\n")[0])
        count -= 1
        if count == 0:
            pass
print "Finished reading."


def tokenize(text):
    text.split(' ')
    return text.split(' ')

vectorizer = TfidfVectorizer(tokenizer=tokenize, min_df=5, max_df=0.03, stop_words='english')
m = vectorizer.fit_transform(corpus)
print "Finished tf-idf.", m.shape[0], m.shape[1]

print vectorizer.get_stop_words()
phrases = vectorizer.get_feature_names()
with open(output_file, 'w') as output:
    for i in xrange(m.shape[0]):
        d = m.getrow(i)
        s = zip(d.indices, d.data)
        sorted_s = sorted(s, key=lambda v: v[1], reverse=True)
        indices = [element[0] for element in sorted_s]
        for i in range(min(top_K, len(indices))):
            output.write(phrases[indices[i]])
            output.write(' ')
        output.write('\n')
