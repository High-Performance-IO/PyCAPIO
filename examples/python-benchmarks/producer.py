import argparse
import os
import sys
import time

from pycapio.context import CapioContext


def benchmark_writer(filename, size_mb):
    data = os.urandom(1024 * 1024)  # 1MB buffer
    start_time = time.perf_counter()

    with open(filename, 'wb') as f:
        for _ in range(size_mb):
            f.write(data)

    end_time = time.perf_counter()
    duration = end_time - start_time
    throughput = size_mb / duration

    print(f"WRITE: {size_mb} MB - Time: {duration:.4f} seconds - Speed: {throughput:.2f} MB/s")


@CapioContext(capio_dir="/mnt/lustre/e1000/home/dc167/dc167/msantima/libcapio/examples/python-benchmarks", 
              capio_app_name="consumer", 
              capio_workflow_name="benchmark")
def capio_benchmark_writer(filename, size_mb):
    benchmark_writer(filename, size_mb)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Direct I/O Benchmark")
    parser.add_argument("--file", type=str, required=True, help="Filename to use")
    parser.add_argument("--size", type=int, required=True, help="Size in MB")
    parser.add_argument("--capio", action='store_true', required=False)

    args = parser.parse_args()
    if args.capio:
        capio_benchmark_writer(args.file, args.size)
    else:
        benchmark_writer(args.file, args.size)

