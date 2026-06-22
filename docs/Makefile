# Minimal makefile for PyCAPIO documentation.
# Usage:
#   make html   -> Doxygen XML + HTML site in _build/html
#   make pdf     -> Doxygen XML + PDF in _build/latex/pycapio.pdf
#   make clean   -> remove generated output

SPHINXBUILD ?= sphinx-build
SOURCEDIR   = source
BUILDDIR    = _build

.PHONY: help doxygen html pdf latexpdf clean

help:
	@echo "Targets: html, pdf (alias latexpdf), doxygen, clean"

# Generate the C++ XML/HTML used by Breathe.
doxygen:
	@command -v doxygen >/dev/null 2>&1 || { echo "doxygen not found; C++ API pages will be empty"; exit 0; }
	doxygen Doxyfile

html: doxygen
	$(SPHINXBUILD) -b html "$(SOURCEDIR)" "$(BUILDDIR)/html"
	@echo "HTML written to $(BUILDDIR)/html/index.html"

latexpdf pdf: doxygen
	$(SPHINXBUILD) -b latex "$(SOURCEDIR)" "$(BUILDDIR)/latex"
	$(MAKE) -C "$(BUILDDIR)/latex" all-pdf
	@echo "PDF written to $(BUILDDIR)/latex/pycapio.pdf"

clean:
	rm -rf "$(BUILDDIR)" doxygen
