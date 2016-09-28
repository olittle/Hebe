if [[ -z "$INPUT" ]]; then
	INPUT=/shared/data/tensor_embedding/data/dblp/frequent_year
fi

if [[ -z "$OUTPUT" ]]; then
	OUTPUT=results/dblp_noise
fi

# SCORING_FUNCTION=2
if [[ "$SCORING_FUNCTION" -eq "1" ]]; then
	BETA="0 0 0 0 0"
	VALID=10111
else
	BETA="0.05 0.05 0.05 0.05 0.05"
	VALID=11111
fi

if [[ -z "$DIM" ]]; then
	DIM=300
fi

ITER=1500
TENSOR_THREAD=5
WORD2VEC_THREAD=0

LOAD=00000
CHOOSE_TO_OUTPUT=00111
ALPHA=0.05

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
	time ./tensor2vec -type 5 -name term paper author venue year \
	    -tthread $TENSOR_THREAD -wthread $WORD2VEC_THREAD \
		-input $INPUT -output $OUTPUT \
		-choose $CHOOSE_TO_OUTPUT -valid $VALID -iter $ITER -beta $BETA \
		-scoring $SCORING_FUNCTION -load $LOAD -size $DIM -negative 5 -alpha $ALPHA
	echo === evaluating 4 research group labels ===
	evaluate $INPUT/label-group.txt
	echo
	echo === evaluating 4 research area labels ===
	evaluate $INPUT/label-area.txt
	echo
}

run
