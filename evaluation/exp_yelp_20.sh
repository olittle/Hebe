#!/bin/sh
INPUT=/shared/data/tensor_embedding/data/yelp/20percents
OUTPUT=results/yelp20

SESSION_NAME="yelp_exp_20percents"

cd /shared/data/jialu/tensor2vec

tmux has-session -t ${SESSION_NAME}

if [ $? != 0 ]
then
  # Create the session
  tmux new-session -s ${SESSION_NAME} -n vim -d

  # First window (0) -- tensor2vec
  tmux send-keys -t ${SESSION_NAME} "INPUT='$INPUT';export INPUT;OUTPUT='$OUTPUT';export OUTPUT;SCORING_FUNCTION=2;export SCORING_FUNCTION;./run_yelp.sh" C-m
  tmux rename-window -t ${SESSION_NAME} 'tensor2vec'

  # pte, line
  tmux new-window -n pte_n_line -t ${SESSION_NAME}
  tmux send-keys -t ${SESSION_NAME}:1 "cd evaluation/baseline; INPUT='$INPUT';export INPUT;OUTPUT='$OUTPUT';export OUTPUT;HAS_ZIP=0;export HAS_ZIP; ./run_pte_yelp.sh; ./run_line_yelp.sh" C-m

  # svd
  tmux new-window -n svd -t ${SESSION_NAME}
  tmux send-keys -t ${SESSION_NAME}:2 "cd evaluation/baseline; INPUT='$INPUT';export INPUT;OUTPUT='$OUTPUT';export OUTPUT; ./run_svd_yelp.sh" C-m

  # nmf
  tmux new-window -n nmf -t ${SESSION_NAME}
  tmux send-keys -t ${SESSION_NAME}:3 "cd evaluation/baseline; INPUT='$INPUT';export INPUT;OUTPUT='$OUTPUT';export OUTPUT; sleep 10m; ./run_nmf_yelp.sh" C-m

  # scoring function = 1
  tmux new-window -n scoring1 -t ${SESSION_NAME}
  tmux send-keys -t ${SESSION_NAME}:4 "INPUT='$INPUT';export INPUT;OUTPUT='$OUTPUT';export OUTPUT;SCORING_FUNCTION=1;export SCORING_FUNCTION;./run_yelp.sh" C-m

  # Start out on the first window when we attach
  tmux select-window -t ${SESSION_NAME}:0
fi
tmux attach -t ${SESSION_NAME}
