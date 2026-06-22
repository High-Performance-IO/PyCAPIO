# PyCAPIO documentation

This directory contains the [Sphinx](https://www.sphinx-doc.org) documentation
for PyCAPIO. It documents both the Python API (via `autodoc`, reading the
docstrings in `pycapio/`) and the C++ layer (via [Doxygen](https://www.doxygen.nl)
+ [Breathe](https://breathe.readthedocs.io)), and can produce both an HTML site
and a PDF.

## Build locally

Install the documentation dependencies (a virtual environment is recommended):

```console
pip install -r docs/requirements.txt
```

For the C++ API pages you also need Doxygen, and for the PDF you need a LaTeX
toolchain:

```console
# Debian/Ubuntu
sudo apt-get install doxygen texlive-latex-recommended texlive-latex-extra latexmk
# macOS (Homebrew)
brew install doxygen        # and a LaTeX distribution such as MacTeX for PDF
```

Then, from inside `docs/`:

```console
make html     # HTML site  -> docs/_build/html/index.html
make pdf       # PDF        -> docs/_build/latex/pycapio.pdf
make clean     # remove generated output
```

`make` runs Doxygen first (when available) and then Sphinx. The Python pages
build even without Doxygen — only the C++ pages will be empty.

> **Note** — The docs build does **not** require compiling the native
> `pycapio._pycapio` extension. `conf.py` registers a lightweight stand-in for
> it so `autodoc` can import the pure-Python modules anywhere. The native
> classes are documented from the C++ sources instead.

## Writing docstrings

Python docstrings use the [Google style](https://google.github.io/styleguide/pyguide.html#38-comments-and-docstrings)
parsed by `napoleon`. C++ entities use Doxygen comments (`/** ... */` or
`///`). Adding a docstring to a function or class is enough for it to appear in
the rendered API reference — no manual page edits required.
