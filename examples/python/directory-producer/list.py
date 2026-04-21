import grp
import os
import pwd
import stat
import time

from pycapio import CapioContext


@CapioContext(capio_dir="/tmp",
              app_name="consumer",
              workflow_name="list-directory",
              server_exec_path=os.environ["CAPIO_SERVER_BIN"],
              capio_cl_configuration_file=os.environ["CAPIO_CL_CONFIG"])
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
