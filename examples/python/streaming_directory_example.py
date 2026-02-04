import multiprocessing
import os
import sys
import time


def test_streaming():
    from pycapio.context import CapioContext

    @CapioContext(capio_app_name="producer", capio_dir="/tmp", capio_workflow_name="libcapio_readdir_example", silent=False)
    def test_create_dir():
        for i in range(0,4):
            #f = open(f"/tmp/test{i}.txt", "w+")
            #f.close()
            with open(f"/tmp/test{i}.txt", "w+") as f:
                time.sleep(1)


    @CapioContext(capio_app_name="reader", capio_dir="/tmp", capio_workflow_name="libcapio_readdir_example", silent=False)
    def test_read_dir():
        generator = os.scandir("/tmp")
        for file in generator:
            print(f"{file.name} : {file.path}")

    consumer = multiprocessing.Process(target=test_read_dir)

    consumer.start()

    test_create_dir()

    consumer.join()



test_streaming()
