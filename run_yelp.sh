if [[ -z "$INPUT" ]]; then
	INPUT=/shared/data/tensor_embedding/data/yelp/small_tf_idf
fi

if [[ -z "$OUTPUT" ]]; then
	OUTPUT=results/yelp
fi


if [[ "$SCORING_FUNCTION" -eq "1" ]]; then
	VALID_1=101
  VALID_2=01
else
	VALID_1=111
  VALID_2=11
fi

if [[ -z "$DIM" ]]; then
	DIM=300
fi

if [[ -z "$ITER" ]]; then
	ITER=1500
fi

TENSOR_THREAD=20
WORD2VEC_THREAD=0

ALPHA=0.05

FILENAME=yelp_parameters

##############################
RELATION_1="review"
TYPE_1=3
NAME_1="term review business"
CENTER_1=1
WEIGHT_1=1
BETA_1="0 0 0"
LOAD_1=000
VALID_1=111
CHOOSE_TO_OUTPUT_1=001
SCORING_FUNCTION_1=2
##############################
RELATION_2="business"
TYPE_2=2
NAME_2="name relation_id business"
CENTER_2=1
WEIGHT_2=1
BETA_2="0 0 0"
LOAD_2=000
VALID_2=111
CHOOSE_TO_OUTPUT_2=000
SCORING_FUNCTION_2=2
##############################

preprocess() {
	make -s
	if [ ! -e $INPUT/words_edges.txt ]; then
		python preprocess/prepare_word2vec.py -input $INPUT
		chmod a+rwx $INPUT/words_edges.txt
	fi
	mkdir -p $OUTPUT
}

evaluate () {
	THREAD=5
	python evaluation/transform_format.py -emb $OUTPUT/business_embedding.txt -label $1 -output $OUTPUT/data_tensor_norm.txt -norm 1
	echo "logistic regression (normalization):"
	evaluation/liblinear/train -q -s 0 -v 5 -n $THREAD $OUTPUT/data_tensor_norm.txt
	echo "linear svm (normalization):"
	evaluation/liblinear/train -q -s 2 -v 5 -n $THREAD $OUTPUT/data_tensor_norm.txt
	python evaluation/ranking.py -input $OUTPUT/data_tensor_norm.txt

	python evaluation/transform_format.py -emb $OUTPUT/business_embedding.txt -label $1 -output $OUTPUT/data_tensor.txt -norm 0
	echo "logistic regression (no normalization):"
	evaluation/liblinear/train -q -s 0 -v 5 -n $THREAD $OUTPUT/data_tensor.txt
	echo "linear svm (no normalization):"
	evaluation/liblinear/train -q -s 2 -v 5 -n $THREAD $OUTPUT/data_tensor.txt
	python evaluation/ranking.py -input $OUTPUT/data_tensor.txt
}

run () {
	preprocess
  
  # first relation type 
  echo -relation $RELATION_1 -weight $WEIGHT_1 -scoring $SCORING_FUNCTION_1 -type $TYPE_1 -center $CENTER_1 -name $NAME_1 -beta $BETA_1 -load $LOAD_1 -valid $VALID_1 -load $LOAD_1 -choose $CHOOSE_TO_OUTPUT_1 > $FILENAME 
  # second relation type  
  echo -relation $RELATION_2 -weight $WEIGHT_2 -scoring $SCORING_FUNCTION_2 -type $TYPE_2 -center $CENTER_2 -name $NAME_2 -beta $BETA_2 -load $LOAD_2 -valid $VALID_2 -load $LOAD_2 -choose $CHOOSE_TO_OUTPUT_2 >> $FILENAME 

  echo -para $FILENAME \
    -tthread $TENSOR_THREAD -wthread $WORD2VEC_THREAD \
		-input $INPUT -output $OUTPUT \
		-iter $ITER -size $DIM -negative 5 -alpha $ALPHA
  

  time ./tensor2vec -para $FILENAME \
    -tthread $TENSOR_THREAD -wthread $WORD2VEC_THREAD \
		-input $INPUT -output $OUTPUT \
		-scoring $SCORING_FUNCTION -iter $ITER -size $DIM -negative 5 -alpha $ALPHA
  
	echo === evaluating 11 business category labels ===
	evaluate $INPUT/label-business.txt
	echo
}

run

