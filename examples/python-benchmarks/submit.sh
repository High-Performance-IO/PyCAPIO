#!/bin/bash

#SBATCH --partition=standard
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=3
#SBATCH -o logs/out.log
#SBATCH -e logs/out.log
#SBATCH --job-name=pycapio-bench
#SBATCH --qos=standard
#SBATCH --time=00:20:00
#SBATCH --exclusive

CAPIO_CL_CONF_PATH=""
SIZES=(1 2 4 8 16 32 64 128 256 512 1024 2048)
TEST_ITER=3

CONS_TASK_COUNT=$((SLURM_NTASKS - 2))

for SIZE in "${SIZES[@]}"; do

    TEST_FILENAME="test_file_${SIZE}MB.bin"
    srun --nodes=1 --ntasks=1 --overlap capio_server -c "$CAPIO_CL_CONF_PATH" &
    SERVER_PID=$!

    for _ in $(seq 1 $TEST_ITER); do
      srun --nodes=1 --ntasks=$CONS_TASK_COUNT --overlap python3 consumer.py --file "$TEST_FILENAME" &
      CONS_PID=$!
      srun  --nodes=1 --ntasks=1 --overlap  python3 producer.py --file "$TEST_FILENAME" --size_mb "$SIZE"
      wait $CONS_PID

      rm -f "$TEST_FILENAME"
    done

    killall $SERVER_PID
    rm -rf files_locations*.txt
    rm -rf /dev/shm/*
    rm -rf capio_logs

done