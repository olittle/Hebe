if [[ -z "$INPUT" ]]; then
	INPUT=/shared/data/tensor_embedding/data/dblp/frequent
fi

if [[ -z "$OUTPUT" ]]; then
	OUTPUT=results/dblp
fi

if [[ -z "$DIM" ]]; then
	DIM=300
fi

if [[ -z "$ITER" ]]; then
	ITER=1500
fi

TENSOR_THREAD=20
WORD2VEC_THREAD=0

CHOOSE_TO_OUTPUT=1011
ALPHA=0.05

FILENAME=dblp_parameters

##############################
RELATION_1="paper"
TYPE_1=4
NAME_1="term paper author venue"
CENTER_1=1
WEIGHT_1=1
LOAD_1=0000
SCORING_FUNCTION_1=2
if [[ "$SCORING_FUNCTION_1" -eq "1" ]]; then
	BETA_1="0 0 0 0"
	VALID_1=1011
else
	BETA_1="0.05 0.05 0.05 0.05"
	VALID_1=1111
fi
CHOOSE_TO_OUTPUT_1=0011
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
	python evaluation/transform_format.py -emb $OUTPUT/author_embedding.txt -label $1 -output $OUTPUT/data_tensor_norm.txt -norm 1
	echo "logistic regression (normalization):"
	evaluation/liblinear/train -q -s 0 -v 5 -n $THREAD $OUTPUT/data_tensor_norm.txt
	echo "linear svm (normalization):"
	evaluation/liblinear/train -q -s 2 -v 5 -n $THREAD $OUTPUT/data_tensor_norm.txt
	python evaluation/ranking.py -input $OUTPUT/data_tensor_norm.txt

	python evaluation/transform_format.py -emb $OUTPUT/author_embedding.txt -label $1 -output $OUTPUT/data_tensor.txt -norm 0
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
  
	time ./tensor2vec -para $FILENAME \
	    -tthread $TENSOR_THREAD -wthread $WORD2VEC_THREAD \
		-input $INPUT -output $OUTPUT \
		-iter $ITER -size $DIM -negative 5 -alpha $ALPHA
	echo === evaluating 4 research group labels ===
	evaluate $INPUT/label-group.txt
	echo
	echo === evaluating 4 research area labels ===
	evaluate $INPUT/label-area.txt
	echo
}

run
