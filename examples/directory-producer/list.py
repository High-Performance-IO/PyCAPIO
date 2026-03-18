import grp
import os
import pwd
import stat
import time

from pycapio.context import CapioContext


@CapioContext(capio_dir="/tmp", capio_app_name="consumer", capio_workflow_name="list-directory")
def list_directory_raw():
    try:
        for entry in os.scandir('/tmp'):
            name = entry.name
            if entry.is_symlink():
                try:
                    link_target = os.readlink(entry.path)
                    name = f"{name} -> {link_target}"
                except OSError as e:
                    print(e)
                    pass

            print(f"{name}")

    except PermissionError:
        print("Error: Permission denied to access this directory.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")


if __name__ == "__main__":
    list_directory_raw()
