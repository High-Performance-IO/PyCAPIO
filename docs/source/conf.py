# Configuration file for the Sphinx documentation builder.
#
# Full reference: https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import sys
import shutil
import subprocess
import types
from pathlib import Path

# -- Paths -------------------------------------------------------------------
# Make the `pycapio` package importable by autodoc.
#   CONF_DIR  = docs/source   (this file's directory; the Sphinx source root)
#   DOCS_DIR  = docs          (holds the Doxyfile and Doxygen output)
#   REPO_ROOT = repository root
CONF_DIR = Path(__file__).resolve().parent
DOCS_DIR = CONF_DIR.parent
REPO_ROOT = DOCS_DIR.parent
sys.path.insert(0, str(REPO_ROOT))

# -- Stub the compiled extension --------------------------------------------
# `pycapio` does `from ._pycapio import *` at import time. `_pycapio` is a
# pybind11 extension that is only available after a full native build (MPI,
# CMake, CAPIO). To let the docs build anywhere -- locally, on GitHub Actions,
# on Read the Docs -- without that toolchain, we register a lightweight stand-in
# for `pycapio._pycapio` *before* autodoc imports the package.
#
# The pure-Python API (CapioContext, the *_proxy functions, the CLI launcher)
# is then documented from the real source via autodoc, while the native classes
# and functions are documented from the C++ sources via Breathe/Doxygen.


def _install_native_stub() -> None:
    stub = types.ModuleType("pycapio._pycapio")

    # Constants referenced at import time (default arguments, etc.).
    stub.CAPIO_DEFAULT_APP_NAME = "writer"
    stub.CAPIO_DEFAULT_WORKFLOW_NAME = "workflow"
    stub.FILE_MODES = {
        "O_RDONLY": 0,
        "O_WRONLY": 1,
        "O_RDWR": 2,
        "O_CREAT": 64,
        "O_APPEND": 1024,
    }

    def _make_callable(name):
        def _f(*args, **kwargs):  # pragma: no cover - never executed
            raise RuntimeError(
                f"{name} is a native symbol stubbed out for the docs build"
            )

        _f.__name__ = name
        return _f

    for fn in (
        "pycapio_init",
        "pycapio_teardown",
        "pycapio_open",
        "pycapio_mkdir",
        "pycapio_get_capio_dir",
    ):
        setattr(stub, fn, _make_callable(fn))

    for cls in (
        "PyCapioTextIOWrapper",
        "PyCapioBinaryIOWrapper",
        "PyCapioScandirWrapper",
        "PyCapioPath",
        "DirEntry",
        "PyCAPIOException",
    ):
        setattr(stub, cls, type(cls, (), {}))

    # Make `from ._pycapio import *` populate every public name.
    stub.__all__ = [n for n in vars(stub) if not n.startswith("_")]

    sys.modules["pycapio._pycapio"] = stub


_install_native_stub()

# -- Project information -----------------------------------------------------
project = "PyCAPIO"
copyright = "2026, Alpha Parallel Computing Research Group, University of Torino, Italy."
author = "Marco Edoardo Santimaria"

# -- Version -----------------------------------------------------------------
# Read the version straight from CMakeLists.txt, mirroring the regex used by
# scikit-build-core in pyproject.toml. This works without installing the
# package (which the docs build deliberately avoids).
import re


def _read_version() -> str:
    cmakelists = REPO_ROOT / "CMakeLists.txt"
    try:
        text = cmakelists.read_text(encoding="utf-8")
        m = re.search(
            r"(?m)VERSION\s+(?P<major>\d+)\.(?P<minor>\d+)\.(?P<patch>\d+)",
            text,
        )
        if m:
            return f"{m['major']}.{m['minor']}.{m['patch']}"
    except OSError:
        pass
    # Fallback: an installed package, if present.
    try:
        from importlib.metadata import version as _pkg_version

        return _pkg_version("pycapio")
    except Exception:
        return "0.0.0"


release = _read_version()
version = ".".join(release.split(".")[:2])

# -- General configuration ---------------------------------------------------
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
    "sphinx.ext.intersphinx",
    "myst_parser",
    "breathe",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store", "README.md"]

# Accept both Markdown (MyST) and reStructuredText sources.
source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

# -- Autodoc / Napoleon ------------------------------------------------------
autosummary_generate = True
autodoc_member_order = "bysource"
autodoc_typehints = "description"
autodoc_default_options = {
    "members": True,
    "undoc-members": True,
    "show-inheritance": True,
}
# Any genuinely external module that might be imported. The native extension is
# handled by the stub above; list other heavy deps here.
autodoc_mock_imports = ["py_capio_cl"]

napoleon_google_docstring = True
napoleon_numpy_docstring = False
napoleon_include_init_with_doc = True
napoleon_use_rtype = True

# -- Intersphinx -------------------------------------------------------------
intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
}

# -- MyST --------------------------------------------------------------------
myst_enable_extensions = ["colon_fence", "deflist"]
myst_heading_anchors = 3

# -- Breathe (C++ via Doxygen) ----------------------------------------------
breathe_projects = {"pycapio": str(DOCS_DIR / "doxygen" / "xml")}
breathe_default_project = "pycapio"
breathe_default_members = ("members",)


def _run_doxygen() -> None:
    """Generate Doxygen XML so Breathe has something to read.

    Runs automatically when the XML output is missing (for example on
    Read the Docs, where there is no separate build step). Silently does
    nothing if the ``doxygen`` executable is unavailable.
    """
    xml_index = DOCS_DIR / "doxygen" / "xml" / "index.xml"
    if xml_index.exists():
        return
    if shutil.which("doxygen") is None:
        print("WARNING: 'doxygen' not found; the C++ API pages will be empty.")
        return
    print("Running Doxygen to generate C++ XML ...")
    subprocess.call(["doxygen", "Doxyfile"], cwd=str(DOCS_DIR))


_run_doxygen()

# -- HTML output -------------------------------------------------------------
html_theme = "furo"
html_static_path = ["_static"]
html_title = f"PyCAPIO {release}"
html_theme_options = {
    "source_repository": "https://github.com/High-Performance-IO/PyCAPIO",
    "source_branch": "main",
    "source_directory": "docs/",
}

# -- LaTeX / PDF output ------------------------------------------------------
latex_engine = "pdflatex"
latex_elements = {
    "papersize": "a4paper",
    "pointsize": "11pt",
    "preamble": r"\usepackage{enumitem}\setlistdepth{99}",
}
latex_documents = [
    ("index", "pycapio.tex", "PyCAPIO Documentation", author, "manual"),
]
