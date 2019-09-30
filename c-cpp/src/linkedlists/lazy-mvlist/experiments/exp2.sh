#! /bin/bash

LISTS="${MVL_UNSAFE} ${MVL_MVLIST}"
SIZES="1024 5000 16384 100000"
THREADS="1 8 16 24 32 40 48"
UPDATE_RATES="20 50 100"
MAX_RQS="4 8 16"
RQ_THREADS="ALL"
RQ_RATES="100"
NUMA_FLAGS="-m0 -N0"
TRIALS=10
DURATION=500
declare -a GREP=("${MVL_RQ_OPS}" "${MVL_UPDATE_OPS}" "${MVL_ALL_OPS}")

cd $MVL_SRC_DIR
make -s NOPREALLOC=TRUE
cd $MVL_EXP_DIR