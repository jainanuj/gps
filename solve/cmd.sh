#!/bin/bash

ulimit -c unlimited

./gps --epsilon=0.0001 \
      --odcd_cache_fn_format=/tmp/odcd_cache-%d \
      --odcd_cache_size=-1 \
      --mdp_fn=../mcar_100x100.mdp \
      --stp_fn=../mcar_20x20.part \
      --run_type=vi \
      --heat_metric=abs \
      --solver=r \
      --use_voting=1 \
      --save_fn=results \
      --verbose=1 \
      $*

