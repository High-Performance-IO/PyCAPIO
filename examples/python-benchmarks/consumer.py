import sys
import time
import argparse
from pycapio.context import CapioContext

def benchmark_reader(filename):
    chunk_size = 1024 * 1024  # 1MB chunk
    total_read = 0

    start_time = time.perf_counter()

    with open(filename, 'rb') as f:
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            total_read += len(chunk)

    end_time = time.perf_counter()
    duration = end_time - start_time
    size_mb = total_read / (1024 * 1024)
    throughput = size_mb / duration

    print(f"READ: {size_mb:.2f} MB - Time: {duration:.4f} seconds - Speed: {throughput:.2f} MB/s")

    return (size_mb, duration, throughput)


@CapioContext(capio_dir="/mnt/lustre/e1000/home/dc167/dc167/msantima/libcapio/examples/python-benchmarks", 
              capio_app_name="consumer", 
              capio_workflow_name="benchmark")
def capio_benchmark_reader(filename):
    return benchmark_reader(filename)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Direct I/O Benchmark")
    parser.add_argument("--file", type=str, required=True, help="Filename to use")
    parser.add_argument("--capio", required=False, action='store_true')

    args = parser.parse_args()

    size, elps, bandwidth = capio_benchmark_reader(args.file) if args.capio else benchmark_reader(args.file)

    with open("result.csv", "a") as r:
        r.write(f"{size},{elps},{bandwidth}\n")
    

