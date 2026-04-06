#!/bin/bash

CAPIO_CL_CONF_PATH="/data/msantimaria/pycapio-tests/libcapio/examples/python-benchmarks/config.json"
SIZES=(1 2 4 8 16 32 64 128 256 512 1024 2048)
TEST_ITER=3

export CAPIO_DIR="/data/msantimaria/pycapio-tests/libcapio/examples/python-benchmarks/capio_dir"

CONS_TASK_COUNT=$((SLURM_NTASKS - 2))

for SIZE in "${SIZES[@]}"; do

    TEST_FILENAME="test_file_${SIZE}MB.bin"
    capio_server -c "$CAPIO_CL_CONF_PATH" &
    SERVER_PID=$!

    for _ in $(seq 1 $TEST_ITER); do
      python3 consumer.py --file "$TEST_FILENAME" --capio &
      CONS_PID=$!
      python3 producer.py --file "$TEST_FILENAME" --capio --size "$SIZE"
      wait $CONS_PID

      rm -f "$TEST_FILENAME"
    done

    killall -ILL $SERVER_PID
    rm -rf files_locations*.txt
    rm -rf /dev/shm/*
    rm -rf capio_logs

done
