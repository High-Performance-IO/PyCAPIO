import sys
import termios
import tty
from datetime import datetime
import os

from pycapio import CapioContext


def get_keypress():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch


@CapioContext(capio_dir="/tmp",
              app_name="producer",
              workflow_name="list-directory",
              server_exec_path=os.environ["CAPIO_SERVER_BIN"],
              capio_cl_configuration_file=os.environ["CAPIO_CL_CONFIG"])
def run_code():
    try:
        while True:
            input("Press any key to produce file")

            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
            filename = f"/tmp/file_{timestamp}.txt"

            with open(filename, 'w'):
                pass
            print(f"\rCreated: {filename}         ")



    except KeyboardInterrupt:
        return


if __name__ == "__main__":
    print("Press [ENTER] to create a unique file.")
    print("Press [CTRL+X] to exit.")
    run_code()
