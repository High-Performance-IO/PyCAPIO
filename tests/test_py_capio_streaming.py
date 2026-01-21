import os
import multiprocessing
import time

os.environ["CAPIO_DIR"] = "/tmp"
os.environ["DEBUG"] = "on"
os.environ["CAPIO_LOG_LEVEL"] = "-1"

file_path = "/tmp/sample_streaming.dat"

def test_streaming():

    def test_write_1gb():
        os.environ["CAPIO_APP_NAME"] = "producer"
        import pycapio
        with open(file_path, "wb") as f:
            f.write(b"1" * 1024 * 1024 * 1024)

    def test_read_1gb():
        os.environ["CAPIO_APP_NAME"] = "consumer"
        import pycapio
        with open(file_path, "rb") as f:
            data = f.read()
        for char in data:
            assert char == "1"

    consumer = multiprocessing.Process(target=test_read_1gb)
    producer = multiprocessing.Process(target=test_write_1gb)

    start_time = time.perf_counter()
    consumer.start()
    producer.start()

    producer.join()
    consumer.join()
    end_time = time.perf_counter()
    elapsed = end_time - start_time
    print(f"Total time elapsed: {elapsed:.4f} seconds")

