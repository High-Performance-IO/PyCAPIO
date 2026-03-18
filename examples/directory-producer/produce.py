import sys
import tty
import termios
from datetime import datetime

from pycapio.context import CapioContext

def get_keypress():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch


@CapioContext(capio_dir="/tmp", capio_app_name="producer", capio_workflow_name="list-directory")
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