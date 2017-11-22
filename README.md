Compilers
=========

Introduction
------------

This is an educational project. The goal is to write simple interpreter
and compiler.

Directory structure:
```
doc/      - Notes on writing compiler
src/      - Sources of the compiler/interpreter
examples/ - Example programs for the compiler
```

Requirements
------------

To build compiler/interpreter you need:
- flex
- bison

Compiler produces assembler code in [flatassembler](http://flatassembler.net/) syntax.
You will need fasm in order to translate intermediate code into binaries.

Building documentation requires pdflatex.

Build Instructions
------------------

Run `make`. Check src/ directory for built programs.

Run `make doc` to build documentation.
