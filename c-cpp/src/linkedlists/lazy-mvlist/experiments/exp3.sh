#! /bin/bash

# This experiment measures throughput of two different sizes. All threads are pinned to NUMA 0.
# Only a maximum of 48 threads are used to ensure that all threads are in the same NUMA zone for the target machine, which is a four NUMA, 96-core (192-thread) Intel machine.

LISTS="${MVL_MVLIST}"
SIZES="256 16384"
THREADS="1 8 16 24 32 40 48"
UPDATE_RATES="0 10 20"
MAX_RQS="1 2 4 8 16 32 48"
RQ_THREADS="0"
RQ_RATES="0"
NUMA_FLAGS="-m0 -N0"
DURATION=1000
GREP=$MVL_ALL_OPS

cd $MVL_SRC_DIR
make -s
cd $MVL_EXP_DIR