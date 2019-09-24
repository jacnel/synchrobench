#! /bin/bash

source config.sh setup
if [[ $# -ne 1 ]]; then
  echo -e $MVL_INVALID_NUM_ARGUMENTS
  echo "Usage: $0 <exp-file>."
  exit
fi

source $1

echo -e $MVL_GREEN"----- Configuration parameters -----"$MVL_CLEAR
echo "binaries       : "$LISTS
echo "sizes          : "$SIZES
echo "update rates   : "$UPDATE_RATES
echo "max rqs        : "$MAX_RQS
echo "rq threads     : "$RQ_THREADS
echo "rq rates       : "$RQ_RATES
echo "threads        : "$THREADS
echo "numa flags     : "$NUMA_FLAGS
echo "duration       : "$DURATION
echo -e $MVL_GREEN"------------------------------------"$MVL_CLEAR

echo -e $MVL_GREEN"list, size, update_rate, max_rqs, rq_threads, rq_rate, num_threads, throughput"$MVL_CLEAR
for list in $LISTS; do
  for size in $SIZES; do
    for update in $UPDATE_RATES; do
      for max_rq in $MAX_RQS; do
        for rq_thread in $RQ_THREADS; do
          for rq_rate in $RQ_RATES; do
            if [[ $rq_thread -eq 0 ]] && [[ $rq_rate -gt 0 ]]; then
              continue
            fi
            for thread in $THREADS; do
              for i in {1..10}; do
                echo -n "$list, $size, $update, $max_rq, $rq_thread, $rq_rate, $thread, "
                if [[ $list == $MVL_MVLIST ]]; then
                  numactl $NUMA_FLAGS $MVL_BIN_DIR/$MVL_MVLIST -t$thread -d$DURATION -A -u$update -q$rq_rate -i$size -m$max_rq | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//"
                else
                  numactl $NUMA_FLAGS $MVL_BIN_DIR/$MVL_MVLIST -t$thread -d$DURATION -A -u$update -q$rq_rate -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//"
                fi
              done
            done
          done
        done
        if [[ $list == $MVL_UNSAFE ]]; then
          break # max_rq has no meaning for the unsafe version.
        fi
      done
    done
  done
done

source config.sh teardown
