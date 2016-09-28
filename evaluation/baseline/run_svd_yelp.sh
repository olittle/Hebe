if [[ -z "$INPUT" ]]; then
	INPUT=/shared/data/tensor_embedding/data/yelp/large_tf_idf
fi

if [[ -z "$OUTPUT" ]]; then
	OUTPUT=results/yelp
fi

mkdir -p $INPUT/matlab
if [ ! -e $INPUT/matlab/name0.txt ]; then
	pypy matlab_data_prepare.py -input $INPUT/events.txt -output $INPUT/matlab
	chmod a+rwx -R $INPUT/matlab
fi
mkdir -p $OUTPUT

evaluate () {
	THREAD=5
	python ../transform_format.py -emb $1 -label $2 -output $OUTPUT/data_svd_norm.txt -norm 1
	echo "logistic regression (normalization):"
	../liblinear/train -q -s 0 -v 5 -n $THREAD $OUTPUT/data_svd_norm.txt
	echo "linear svm (normalization):"
	../liblinear/train -q -s 2 -v 5 -n $THREAD $OUTPUT/data_svd_norm.txt
	python ../ranking.py -input $OUTPUT/data_svd_norm.txt

	python ../transform_format.py -emb $1 -label $2 -output $OUTPUT/data_svd.txt -norm 0
	echo "logistic regression (no normalization):"
	../liblinear/train -q -s 0 -v 5 -n $THREAD $OUTPUT/data_svd.txt
	echo "linear svm (no normalization):"
	../liblinear/train -q -s 2 -v 5 -n $THREAD $OUTPUT/data_svd.txt
	python ../ranking.py -input $OUTPUT/data_svd.txt
}

run () {
	matlab -nojvm -nodisplay -nodesktop -r \
		"input_folder='$INPUT/matlab/';output_folder='$OUTPUT';options.dim=$1;options.log=$2;options.norm=$3;run('./collective_svd');quit"
	echo 0 0 > $OUTPUT/svd_embedding$2$3.txt
	paste -d' ' $INPUT/matlab/name2.txt $OUTPUT/svd2.txt >> $OUTPUT/svd_embedding$2$3.txt
	echo === evaluating 11 business category labels ===
	evaluate $OUTPUT/svd_embedding$2$3.txt $INPUT/label-business.txt
	echo
}

echo svd
run 300 0 0
echo svd log
run 300 1 0
echo svd norm
run 300 0 1
echo svd log norm
run 300 1 1
