if [[ $# -ne 2 ]]; then
  echo "Bad number of arguments."
  echo "Usage: $0 <timing_dir> <bin_dir>."
  exit
fi

DIR=$1
if [[ ! -d $DIR ]]; then
  echo "Directory $DIR does not exist."
  exit
fi

BIN_DIR=$2
if [[ ! -d $BIN_DIR ]]; then
  echo "Directory $BIN_DIR does not exist."
  exit
fi

# Some constants for this experiment.
NBTHREADS=8
DUR=200
NUMA=0

# Replace existing macro with NONE to get operation latency.
sed -i 's/#define TIMING .*/#define TIMING NONE/' $DIR/linkedlist-lock.h
THIS_DIR=$PWD
cd $DIR
make
cd $THIS_DIR
MVLIST="$BIN_DIR/MUTEX-lazy-mvlist-timing"

echo -e "\e[091mOverall operation latency...\e[0m"
echo "insert, remove"
for i in 1 2 4 8 16 32 64 128 192; do
  echo "max rq = $i"
  for j in {1..10}; do
    $MVLIST -t$NBTHREADS -d$DUR -A -u20 -q0 -m$i -i1024 | grep -A 2 "total latency" | grep -A 1 "insert" | sed -e "s/.*:[[:space:]]//" | sed -e '1 '"s/$/,/" | tr '\n' ' '
    echo ""
  done
  echo ""
done

# Replace existing macro with NONE to get operation latency.
sed -i 's/#define TIMING .*/#define TIMING ALLOC/' $DIR/linkedlist-lock.h
THIS_DIR=$PWD
cd $DIR
make
cd $THIS_DIR
MVLIST="../../../bin/MUTEX-lazy-mvlist-timing"

echo -e "\e[091mAllocation latency...\e[0m"
echo "insert, remove"
for i in 1 2 4 8 16 32 64 128 192; do
  echo "max rq = $i"
  for j in {1..10}; do
    $MVLIST -t$NBTHREADS -d$DUR -A -u20 -q0 -m$i -i1024 | grep -A 2 "custom latency" | grep -A 1 "insert" | sed -e "s/.*:[[:space:]]//" | sed -e '1 '"s/$/,/" | tr '\n' ' '
    echo ""
  done
  echo ""
done

# Replace existing macro with NONE to get operation latency.
sed -i 's/#define TIMING .*/#define TIMING RECLAIM/' $DIR/linkedlist-lock.h
THIS_DIR=$PWD
cd $DIR
make
cd $THIS_DIR
MVLIST="../../../bin/MUTEX-lazy-mvlist-timing"

echo -e "\e[091mReclaim latency...\e[0m"
echo "insert, remove"
for i in 1 2 4 8 16 32 64 128 192; do
  echo "max rq = $i"
  for j in {1..10}; do
    $MVLIST -t$NBTHREADS -d$DUR -A -u20 -q0 -m$i -i1024 | grep -A 2 "custom latency" | grep -A 1 "insert" | sed -e "s/.*:[[:space:]]//" | sed -e '1 '"s/$/,/" | tr '\n' ' '
    echo ""
  done
  echo ""
done

# Replace existing macro with NONE to get operation latency.
sed -i 's/#define TIMING .*/#define TIMING SET/' $DIR/linkedlist-lock.h
THIS_DIR=$PWD
cd $DIR
make
cd $THIS_DIR
MVLIST="../../../bin/MUTEX-lazy-mvlist-timing"

echo -e "\e[091mSet latency...\e[0m"
echo "insert, remove"
for i in 1 2 4 8 16 32 64 128 192; do
  echo "max rq = $i"
  for j in {1..10}; do
    $MVLIST -t$NBTHREADS -d$DUR -A -u20 -q0 -m$i -i1024 | grep -A 2 "custom latency" | grep -A 1 "insert" | sed -e "s/.*:[[:space:]]//" | sed -e '1 '"s/$/,/" | tr '\n' ' '
    echo ""
  done
  echo ""
done
