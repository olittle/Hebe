if [[ -z "$INPUT" ]]; then
	INPUT=/shared/data/tensor_embedding/data/yelp/large_tf_idf_zip
fi

if [[ -z "$OUTPUT" ]]; then
	OUTPUT=results/yelp
fi

# HAS_ZIP=1
if [[ "$HAS_ZIP" -eq "1" ]]; then
  TYPE_NAMES=term_review_busiid_businame_zip
else
  TYPE_NAMES=term_review_busiid_businame
fi

PTE_PATH=./pte
# options = main_dblp_heter.cpp, main_dblp_homo.cpp, main_yelp_heter.cpp, main_yelp_homo.cpp
MAIN=$PTE_PATH/main_yelp_heter.cpp
CENTER_TYPE=1
IS_HETER=1
RESULT_VEC=$OUTPUT/pte_vec.txt$IS_HETER
mkdir -p $OUTPUT
cp $MAIN $PTE_PATH/main.cpp
cd $PTE_PATH
make clean
make -s
cd -

if [ ! -d "$INPUT/pte" ]; then
	mkdir -p $INPUT/pte
    python convert2pte.py $INPUT/events.txt $INPUT/pte/ $TYPE_NAMES $CENTER_TYPE $IS_HETER
    chmod -R a+rwx $INPUT/pte
fi

evaluate () {
	THREAD=5
	python ../transform_format.py -emb $1 -label $2 -output $OUTPUT/data_pte_norm.txt -norm 1
	echo "logistic regression (normalization):"
	../liblinear/train -q -s 0 -v 5 -n 5 $OUTPUT/data_pte_norm.txt
	echo "linear svm (normalization):"
	../liblinear/train -q -s 2 -v 5 -n 5 $OUTPUT/data_pte_norm.txt
	python ../ranking.py -input $OUTPUT/data_pte_norm.txt

	python ../transform_format.py -emb $1 -label $2 -output $OUTPUT/data_pte.txt -norm 0
	echo "logistic regression (no normalization):"
	../liblinear/train -q -s 0 -v 5 -n 5 $OUTPUT/data_pte.txt
	echo "linear svm (no normalization):"
	../liblinear/train -q -s 2 -v 5 -n 5 $OUTPUT/data_pte.txt
	python ../ranking.py -input $OUTPUT/data_pte.txt
}

run () {
	echo start training pte
	$PTE_PATH/pte -path $INPUT/pte/ -output $RESULT_VEC -binary 0 -size $1 -negative 5 -samples $2 -threads 20 -zip $HAS_ZIP
	echo === evaluating 11 business categories labels ===
	evaluate $RESULT_VEC $INPUT/label-business.txt
	echo
}

run 300 1000
