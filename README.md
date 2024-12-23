# vole-sim

> Vole Machine Simulator & GUI

**Live Demo:** https://faresbakhit.github.io/vole-sim/

![](./misc/images/screenshot1.png)

Compiles & works on Linux, Windows, and the Web via WebAssembly/Emscripten.

## Building

**Dependencies**: CMake 3.5 or later and the headers and libraries for your OS and window system. SDL2 and FreeType development packages need to be installed
to build the GUI.

```sh
$ git clone --recurse-submodules git@github.com:faresbakhit/vole-sim.git
$ cd vole-sim/
$ cmake -DCMAKE_BUILD_TYPE=Release -B build/
$ cmake --build build/
$ ls build/
...
vole-sim
vole-sim-gui
```

## Materials

- [vole-isa.pdf](./materials/vole-isa.pdf): The Vole architecture and machine language specification from the appendix of the book, Computer Science: An Overview by J. Glenn Brookshear, Dennis Brylow.
- [comp-arch.pdf](./materials/comp-arch.pdf): The 2nd chapter of the same book, an introduction to concepts of computer architecture, which is required to implement a machine such as Vole.
