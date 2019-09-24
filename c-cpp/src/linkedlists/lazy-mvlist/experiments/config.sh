# Configuration for experiments. This should be passed as an argument to `source` so that the environment variables are visible.

# Echo formatting variables. Used for error messaging.
MVL_RED="\e[091m"
MVL_GREEN="\e[32m"
MVL_CLEAR="\e[0m"
MVL_INVALID_NUM_ARGS="${RED}Invalid number of arguments.${CLEAR}"

if [[ $# -ne 1 ]]; then
  echo -e ${INVALID_NUM_ARGS}
  echo "Usage: $0 <setup|teardown>."
fi

# Source directory for MVList and competitors/alternatives.
MVL_EXP_DIR=$PWD
MVL_SRC_DIR=..
MVL_COMP_DIR=../competitors

# Directories of competitors/alternatives used in the experiments.
MVL_MVLIST=MUTEX-lazy-mvlist
MVL_LAZY=MUTEX-lazy-list
MVL_UNSAFE=MUTEX-lazy-list-unsafe
MVL_TIMING=MUTEX-lazy-list-timing

# Directory for all compiled binaries.
MVL_BIN_DIR=../../../../bin

# Directory where to store results.
MVL_RESULTS_DIR=./results
