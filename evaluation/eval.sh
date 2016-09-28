PYTHON=pypy
FOLDER=../output/1111-weight/
THREAD=5

# # For ranking AUC
# $PYTHON ranking.py -emb $FOLDER/embedding_author.txt -label labels/label-group.txt
# $PYTHON ranking.py -emb $FOLDER/embedding_author.txt -label labels/label-area.txt

# # For analogy
# $PYTHON analogy.py -input $FOLDER
# # sample query:
# # venue:kdd - term:data_mining + term:databases
# # author:jiawei_han - venue:kdd + venue:nips
# # venue:kdd - term:data_mining + term:computer_vision

# For node classification
mkdir -p tmp
$PYTHON transform_format.py -emb $FOLDER/author_embedding.txt -label labels/label-area.txt -output tmp/data.txt
echo logistic regression:
liblinear/train -q -s 0 -v 5 -n $THREAD tmp/data.txt
echo l2-loss svm:
liblinear/train -q -s 2 -v 5 -n $THREAD tmp/data.txt

