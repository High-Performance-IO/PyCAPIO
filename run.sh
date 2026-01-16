#!/usr/bin/bash

export DEBUG=ON
export CAPIO_LOG_LEVEL=-1


rm -rf capio_logs /dev/shm/* files_location* PyCapio.log

PYCAPIO_VENV_PATH=/home/marco/Desktop/py-capio
CAPIO_SERVER_PATH=/home/marco/Desktop/capio/cmake-build-release/capio/server/capio_server
BINARY_PATH=/home/marco/Desktop/libcapio/cmake-build-release/libcapio_example

SIZE=67108864

start_time=$(date +%s%N)

CAPIO_APP_NAME=writer \
CAPIO_DIR=/dev \
$BINARY_PATH --write "$SIZE"

CAPIO_APP_NAME=reader \
CAPIO_DIR=/dev \
$BINARY_PATH --read "$SIZE"

end_time=$(date +%s%N)
elapsed_nanos=$((end_time - start_time))

elapsed_ms=$((elapsed_nanos / 1000000))

echo "NO CAPIO took: $elapsed_ms ms"


CAPIO_DIR=/tmp CAPIO_LOG_LEVEL=-1 $CAPIO_SERVER_PATH -c example_config.json &
SERVER_PID=$!

sleep 1
export CAPIO_WORKFLOW_NAME=libcapio_example

start_time=$(date +%s%N)

CAPIO_APP_NAME=writer \
$BINARY_PATH --read "$SIZE" &
PID=$!

CAPIO_APP_NAME=reader \
$BINARY_PATH --write "$SIZE"

wait $PID

end_time=$(date +%s%N)
elapsed_nanos=$((end_time - start_time))

elapsed_ms=$((elapsed_nanos / 1000000))

echo "CAPIO took: $elapsed_ms ms"

kill $SERVER_PID

rm -rf /dev/shm/*