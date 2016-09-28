#!/bin/sh
INPUT=/shared/data/tensor_embedding/data/yelp/large_tf_idf
OUTPUT=results/yelp_dim

SESSION_NAME="yelp_dim_exp"

cd /shared/data/jialu/tensor2vec

tmux has-session -t ${SESSION_NAME}

if [ $? != 0 ]
then
  # Create the session
  tmux new-session -s ${SESSION_NAME} -n vim -d

  # First window (0) -- 100 dim
  tmux send-keys -t ${SESSION_NAME} "INPUT='$INPUT';export INPUT;OUTPUT='2$OUTPUT$DIM';export OUTPUT;DIM=100;export DIM;SCORING_FUNCTION=2;export SCORING_FUNCTION;./run_yelp.sh" C-m
  tmux rename-window -t ${SESSION_NAME} '100score2'

  # 200 dim
  tmux new-window -n 200score2 -t ${SESSION_NAME}
  tmux send-keys -t ${SESSION_NAME}:1 "INPUT='$INPUT';export INPUT;OUTPUT='2$OUTPUT$DIM';export OUTPUT;DIM=200;export DIM;SCORING_FUNCTION=2;export SCORING_FUNCTION;./run_yelp.sh" C-m

  # 400 dim
  tmux new-window -n 400score2 -t ${SESSION_NAME}
  tmux send-keys -t ${SESSION_NAME}:2 "INPUT='$INPUT';export INPUT;OUTPUT='2$OUTPUT$DIM';export OUTPUT;DIM=400;export DIM;SCORING_FUNCTION=2;export SCORING_FUNCTION;./run_yelp.sh" C-m

  # 500 dim
  tmux new-window -n 500score2 -t ${SESSION_NAME}
  tmux send-keys -t ${SESSION_NAME}:3 "INPUT='$INPUT';export INPUT;OUTPUT='2$OUTPUT$DIM';export OUTPUT;DIM=500;export DIM;SCORING_FUNCTION=2;export SCORING_FUNCTION;./run_yelp.sh" C-m

  # 100 dim
  tmux new-window -n 100score1 -t ${SESSION_NAME}
  tmux send-keys -t ${SESSION_NAME}:4 "INPUT='$INPUT';export INPUT;OUTPUT='1$OUTPUT$DIM';export OUTPUT;DIM=100;export DIM;SCORING_FUNCTION=1;export SCORING_FUNCTION;./run_yelp.sh" C-m

  # 200 dim
  tmux new-window -n 200score1 -t ${SESSION_NAME}
  tmux send-keys -t ${SESSION_NAME}:5 "INPUT='$INPUT';export INPUT;OUTPUT='1$OUTPUT$DIM';export OUTPUT;DIM=200;export DIM;SCORING_FUNCTION=1;export SCORING_FUNCTION;./run_yelp.sh" C-m

  # 400 dim
  tmux new-window -n 400score1 -t ${SESSION_NAME}
  tmux send-keys -t ${SESSION_NAME}:6 "INPUT='$INPUT';export INPUT;OUTPUT='1$OUTPUT$DIM';export OUTPUT;DIM=400;export DIM;SCORING_FUNCTION=1;export SCORING_FUNCTION;./run_yelp.sh" C-m

  # 500 dim
  tmux new-window -n 500score1 -t ${SESSION_NAME}
  tmux send-keys -t ${SESSION_NAME}:7 "INPUT='$INPUT';export INPUT;OUTPUT='1$OUTPUT$DIM';export OUTPUT;DIM=500;export DIM;SCORING_FUNCTION=1;export SCORING_FUNCTION;./run_yelp.sh" C-m

  # Start out on the first window when we attach
  tmux select-window -t ${SESSION_NAME}:0
fi
tmux attach -t ${SESSION_NAME}
