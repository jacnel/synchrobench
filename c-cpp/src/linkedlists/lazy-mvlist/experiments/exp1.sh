#! /bin/bash

# This experiment measures throughput as the number of threads performing operations changes.
# We test two different sizes and three different configurations for the maximum number of range queries.

LISTS="${MVL_UNSAFE} ${MVL_MVLIST}"
SIZES="256 16384"
THREADS="1 8 16 24 32 40 48 56 64 72 80 88 96"
UPDATE_RATES="0 10 20"
MAX_RQS="1 2 4 8 16 24 32 40 48"
RQ_THREADS="0"
RQ_RATES="0"
NUMA_FLAGS="--interleave=all"
DURATION=2000

cd $MVL_SRC_DIR
make -s
cd $MVL_EXP_DIR

cd $MVL_COMP_DIR/lazy-list-unsafe
make -s
cd $MVL_EXP_DIR