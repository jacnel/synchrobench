if [[ $# -ne 2 ]]; then
  echo "Bad number of arguments."
  echo "Usage: $0 <path_to_list> <path_to_mvlist>."
  exit
fi

LIST=$1
if [[ ! -f $LIST ]]; then
  echo "File $LIST does not exist."
  exit
fi

MVLIST=$2
if [[ ! -f $MVLIST ]]; then
  echo "File $MVLIST does not exist."
  exit
fi

# Some constants for this experiment.
NBTHREADS=96
NUMA=-1

for size in 1024 16384; do
  echo -e "\e[91msize = $size\e[0m"

  echo "Getting results for the original lazy list..."
  for i in 0 10 20; do
    if [[ $i -ne 0 ]]; then
      echo ""
    fi
    echo "update=$i"
    for j in {1..10}; do
      if [[ $NUMA -ge 0 ]]; then
          numactl -m$NUMA -N$NUMA $LIST -t$NBTHREADS -A -u$i -q0 -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//"
      else
          $LIST -t$NBTHREADS -A -u$i -q0 -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//"
      fi
    done
  done

  echo "Getting range query results for the unsafe implementation..."
  for i in 0 10 20; do
    if [[ $i -ne 0 ]]; then
      echo ""
    fi
    echo "update=$i"
    echo "1, 2, 4, 8, 16, 32, 64, 128, 192"
    for j in {1..10}; do
      for k in 1 2 4 8 16 32 64 128 192; do
        if [[ $NUMA -ge 0 ]]; then
          if [[ $k -ne 192 ]]; then
            numactl -m$NUMA -N$NUMA $LIST -t$NBTHREADS -A -u$i -q$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//" | sed -e "s/$/,/" | tr '\n' ' '
          else
            numactl -m$NUMA -N$NUMA $LIST -t$NBTHREADS -A -u$i -q$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//"
          fi
        else
          if [[ $k -ne 192 ]]; then
            $LIST -t$NBTHREADS -A -u$i -q$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//" | sed -e "s/$/,/" | tr '\n' ' '
          else
            $LIST -t$NBTHREADS -A -u$i -q$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//"
          fi
        fi
      done
    done
  done

  echo "Getting results for mvlist without any running range queries..."
  for i in 0 10 20; do
    if [[ $i -ne 0 ]]; then
      echo ""
    fi
    echo "update=$i"
    echo "1, 2, 4, 8, 16, 32, 64, 128, 192"
    for j in {1..10}; do
      for k in 1 2 4 8 16 32 64 128 192; do
        if [[ $NUMA -ge 0 ]]; then
          if [[ $k -ne 192 ]]; then
            numactl -m$NUMA -N$NUMA $MVLIST -t$NBTHREADS -A -u$i -q0 -m$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//" | sed -e "s/$/,/" | tr '\n' ' '
          else
            numactl -m$NUMA -N$NUMA $MVLIST -t$NBTHREADS -A -u$i -q0 -m$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//"
          fi
        else
          if [[ $k -ne 192 ]]; then
            $MVLIST -t$NBTHREADS -A -u$i -q0 -m$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//" | sed -e "s/$/,/" | tr '\n' ' '
          else
            $MVLIST -t$NBTHREADS -A -u$i -q0 -m$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//"
          fi
        fi
      done
    done
  done

  echo "Getting results for mvlist while running the maximum number of RQs..."
  for i in 0 10 20; do
    echo "update=$i"
    echo "1, 2, 4, 8, 16, 32, 64, 128, 192"
    for j in {1..10}; do
      for k in 1 2 4 8 16 32 64 128 192; do
        if [[ $NUMA -ge 0 ]]; then
          if [[ $k -ne 192 ]]; then
            numactl -m$NUMA -N$NUMA $MVLIST -t$NBTHREADS -A -u$i -q$k -m$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//" | sed -e "s/$/,/" | tr '\n' ' '
          else
            numactl -m$NUMA -N$NUMA $MVLIST -t$NBTHREADS -A -u$i -q$k -m$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//"
          fi
        else
          if [[ $k -ne 192 ]]; then
            $MVLIST -t$NBTHREADS -A -u$i -q$k -m$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//" | sed -e "s/$/,/" | tr '\n' ' '
          else
            $MVLIST -t$NBTHREADS -A -u$i -q$k -m$k -i$size | grep "#txs" | sed -e "s/.*(//" | sed -e "s/[.].*//"
          fi
        fi
      done
    done
  done
done
