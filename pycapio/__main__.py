import argparse
import os
import runpy
import sys

from pycapio import CapioContext
from ._pycapio import CAPIO_DEFAULT_APP_NAME, CAPIO_DEFAULT_WORKFLOW_NAME


def main():
    parser = argparse.ArgumentParser(description="PyCAPIO module launcher")
    parser.add_argument("--capio-dir", help="CAPIO directory", default=".")
    parser.add_argument("--workflow-name", help="CAPIO workflow name", default=CAPIO_DEFAULT_WORKFLOW_NAME)
    parser.add_argument("--app-name", help="CAPIO app name", default=CAPIO_DEFAULT_APP_NAME)
    parser.add_argument("--capio-cl", help="CAPIO-CL config file path", default="")
    parser.add_argument("script_path", help="Path to the Python module / script to run")
    parser.add_argument("script_args", nargs=argparse.REMAINDER, help="Args for the script")

    args = parser.parse_args()

    @CapioContext(capio_dir=args.capio_dir, workflow_name=args.workflow_name, app_name=args.app_name,
                  teardown_server=True, capio_cl_configuration_file=args.capio_cl)
    def _pycapio_launcher():
        # Update sys.argv so the target script sees its own arguments
        target = args.script_path
        sys.argv = [target] + args.script_args

        print(f"[[PyCAPIO]] CAPIO_DIR={args.capio_dir}")
        print(f"[[PyCAPIO]] CAPIO_APP_NAME={args.app_name}")
        print(f"[[PyCAPIO]] CAPIO_WORKFLOW_NAME={args.workflow_name}")
        print(f"[[PyCAPIO]] Launching \"{' '.join(sys.argv)}\"")

        if os.path.isdir(target):
            # locate standard entry points in a package
            for entry in ["__main__.py", "main.py"]:
                potential_main = os.path.join(target, entry)
                if os.path.exists(potential_main):
                    target = potential_main
                    break

        script_dir = os.path.abspath(os.path.dirname(target))
        if script_dir not in sys.path:
            sys.path.insert(0, script_dir)

        try:
            # try as a path (.py files and dirs with __main__.py)
            runpy.run_path(target, run_name="__main__")
        except (SystemExit, KeyboardInterrupt) as e:
            sys.exit(getattr(e, 'code', 0))
        except Exception as e:
            # try as a module (pip-installed packages)
            print(f"[[PyCAPIO]] Path execution failed: {e}. Trying as module...")
            try:
                runpy.run_module(args.script_path, run_name="__main__", alter_sys=True)
            except SystemExit as e:
                sys.exit(e.code)
            except Exception as mod_e:
                print(f"[[PyCAPIO]] Module execution failed: {mod_e}")
                sys.exit(1)

    _pycapio_launcher()


if __name__ == "__main__":
    main()
