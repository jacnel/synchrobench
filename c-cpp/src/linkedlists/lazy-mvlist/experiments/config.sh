# Configuration for experiments. This should be passed as an argument to `source` so that the environment variables are visible.

# Echo formatting variables. Used for error messaging.
MVL_RED="\e[091m"
MVL_GREEN="\e[32m"
MVL_CLEAR="\e[0m"
MVL_INVALID_NUM_ARGS="${RED}Invalid number of arguments.${CLEAR}"

# Source directory for MVList and competitors/alternatives.
MVL_EXP_DIR=$PWD
MVL_SRC_DIR=..
MVL_COMP_DIR=../competitors

# Directories of competitors/alternatives used in the experiments.
MVL_BIN=MUTEX-lazy-mvlist
MVL_MVLIST=mvlist
MVL_UNSAFE=unsafe

# Directory for all compiled binaries.
MVL_BIN_DIR=../../../../bin

# Directory where to store results.
MVL_RESULTS_DIR=./results

# Strings to grep to get the desired throughput.
MVL_ALL_OPS="#txs"
MVL_RQ_OPS="#rq txs"
MVL_UPDATE_OPS="#update txs"

# NUMA policies.
MVL_NO_NUMA=0
MVL_FILL_NUMA=1
MVL_INTER_NUMA=2

# jemalloc.so path.
MVL_JEMALLOC=/usr/local/lib/libjemalloc.so