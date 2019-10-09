#! /bin/bash

source config.sh
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
echo "numa policy    : "$NUMA_POLICY
echo "duration       : "$DURATION
echo -e $MVL_GREEN"------------------------------------"$MVL_CLEAR

echo -n "list,size,update_rate,max_rqs,rq_threads,rq_rate,num_threads"
i=0
while [[ $i -lt ${#GREP[@]} ]]; do
  echo -n ","
  echo -n ${GREP[i]}
  i=$(($i + 1))
done
echo

# Run experiments for each configuration.
for list in $LISTS; do
  for size in $SIZES; do
    for update in $UPDATE_RATES; do
      for max_rq in $MAX_RQS; do
        # TODO: Make more flexible.
        if [[ $RQ_THREADS -eq "ALL" ]]; then
          RQ_THREADS_=$(seq 0 $max_rq)
        else
          RQ_THREADS_=$RQ_THREADS
        fi
        for rq_thread in $RQ_THREADS_; do
          for rq_rate in $RQ_RATES; do
            # If we are not performing any RQs, then just skip iteration.
            if [[ $rq_thread -gt 0 ]] && [[ $rq_rate -eq 0 ]]; then
              continue
            fi
            for thread in $THREADS; do
              # Skip iteration if there are more requested RQ threads than total threads.
              if [[ $rq_thread -gt $thread ]]; then
                continue
              fi

              # Create an array to store intermediate results of each desired output.
              avgs=()
              i=0
              while [[ $i -lt ${#GREP[@]} ]]; do
                avgs+=(0)
                i=$(($i + 1))
              done

              i=0
              echo -n "$list,$size,$update,$max_rq,$rq_thread,$rq_rate,$thread"
              while [[ $i -lt $TRIALS ]]; do
                # Run trial and collect the results.
                if [[ $list == $MVL_MVLIST ]]; then
                  $MVL_BIN_DIR/$MVL_BIN -f0 -t$thread -d$DURATION -u$update -q$rq_thread -R$rq_rate -i$size -r$(($size * 2)) -m$max_rq -n${NUMA_POLICY} &>temp
                else
                  $MVL_BIN_DIR/$MVL_BIN -f0 -t$thread -d$DURATION -u$update -q$rq_thread -R$rq_rate -i$size -r$(($size * 2)) -U -n${NUMA_POLICY} &>temp
                fi

                # Grep for the desired result and accumulate for averaging.
                j=0
                # echo "${OUTPUT}"
                while [[ $j -lt ${#GREP[@]} ]]; do
                  result=$(cat temp | grep "${GREP[j]}" | sed -e "s/.*(//" | sed -e "s/[.].*//")
                  re='^[-+]?[0-9]+([.][0-9]+)?$'
                  if [[ -z "${result}" ]]; then
                    echo -e $MVL_RED"\nA problem occured while grepping for ${GREP[j]}."$MVL_CLEAR
                    echo "${OUTPUT}"
                    exit
                  fi
                  avgs[j]=$((${avgs[j]} + $result))
                  j=$(($j + 1))
                done
                i=$(($i + 1))
              done

              # Calculate average and output results.
              i=0
              while [[ $i -lt ${#GREP[@]} ]]; do
                avgs[i]=$(echo - | awk "{printf \"%4.1f\",${avgs[i]} / $TRIALS}")
                echo -n ","
                echo -n ${avgs[i]}
                i=$(($i + 1))
              done
              echo
            done
          done
        done
      done
    done
  done
done
