#!/bin/bash

. /home/marco/Desktop/libcapio/.venv/bin/activate

export CAPIO_CL_CONFIG=/home/marco/Desktop/libcapio/examples/directory-producer/config.json
export CAPIO_SERVER_BIN=/home/marco/Desktop/capio/cmake-build-debug/capio/server/capio_server


python3 list.py &
python3 produce.py