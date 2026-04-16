import argparse
import importlib.util
import os
import sys

from pycapio import CapioContext
from ._pycapio import CAPIO_DEFAULT_APP_NAME, CAPIO_DEFAULT_WORKFLOW_NAME


def main():
    parser = argparse.ArgumentParser(description="PyCAPIO module launcher")
    parser.add_argument("--capio_dir", help="CAPIO directory", default=".")
    parser.add_argument("--workflow_name", help="CAPIO workflow name", default=CAPIO_DEFAULT_WORKFLOW_NAME)
    parser.add_argument("--app_name", help="CAPIO app name", default=CAPIO_DEFAULT_APP_NAME)
    parser.add_argument("script_path", help="Path to the Python module / script to run")
    parser.add_argument("script_args", nargs=argparse.REMAINDER, help="Args for the script")

    args = parser.parse_args()

    @CapioContext(capio_dir=args.capio_dir, workflow_name=args.workflow_name, app_name=args.app_name,
                  teardown_server=True)
    def launcher():
        sys.argv = [args.script_path] + args.script_args
        abs_path = os.path.abspath(args.script_path)
        spec = importlib.util.spec_from_file_location("__main__", abs_path)

        if spec is None or spec.loader is None:
            print(f"[[PyCAPIO]] Error: module {abs_path} does not contain main()")
            sys.exit(1)

        module = importlib.util.module_from_spec(spec)
        sys.modules["__main__"] = module

        try:
            print(f"[[PyCAPIO]] Launching {args.script_path}")
            spec.loader.exec_module(module)
        except SystemExit as e:
            sys.exit(e.code)
        except Exception as e:
            print(f"[[PyCAPIO]] Execution Error: {e}")
            sys.exit(1)

    launcher()


if __name__ == "__main__":
    main()
