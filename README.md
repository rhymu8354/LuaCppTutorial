# LuaCppTutorial

This is an example workspace demonstrating various aspects of how to use Lua
in C++ programs.  It was designed to accompany the video,
[Tutorial: Using Lua with C++](https://example.com/put-url-to-youtube-video-here).

## Usage

The `Example1` program is the most basic example and just demonstrates calling
Lua to perform a simple computation.  It should print the answer `42` which is
computed in Lua as the sum of the numbers `15` and `27`.

The `Example2` program performs the same work as `Example`, but demonstrates
how to use custom delegates to more finely control how the program interacts
with Lua.

The `Example3` program demonstrates how to create userdata values in Lua in
order to encapsulate C++ values, both simple and complex.

The `Example4` program demonstrates how to use the Lua registry to capture Lua
values and keep them in C++ for later use with Lua.

## Supported platforms / recommended toolchains

This is a collection of portable C++11 libraries and programs which depends on
the C++11 compiler and standard C and C++ libraries.  It should be supported on
almost any platform.  The following are recommended toolchains for popular
platforms.

* Windows -- [Visual Studio](https://www.visualstudio.com/) (Microsoft Visual
  C++)
* Linux -- clang or gcc
* MacOS -- Xcode (clang)

## Building

This project can stand alone or be included in larger projects.  The Lua source
code is pulled in via `git submodule` from a separate Git repository.
[CMake](https://cmake.org/) files are included for your convenience to generate
a build system to compile the source code and link them into programs you can
run.

There are two distinct steps in the build process using CMake:

1. Generation of the build system, using CMake
2. Compiling, linking, etc., using CMake-compatible toolchain

### Prerequisites

* [CMake](https://cmake.org/) version 3.8 or newer
* C++11 toolchain compatible with CMake for your development platform (e.g.
  [Visual Studio](https://www.visualstudio.com/) on Windows)
* [LuaLibrary](https://github.com/rhymu8354/lua.git) - a fork of
  [Lua](http://www.lua.org/) which includes a `CMakeLists.txt` file for
  incorporation into a CMake-based build system; this repository can be pulled
  in by running the command `git submodule update` from the directory
  containing this file

### Build system generation

Generate the build system using [CMake](https://cmake.org/) from the solution
root.  For example:

```bash
mkdir build
cd build
cmake -G "Visual Studio 15 2017" -A "x64" ..
```

### Compiling, linking, et cetera

Either use [CMake](https://cmake.org/) or your toolchain's IDE to build.
For [CMake](https://cmake.org/):

```bash
cd build
cmake --build . --config Release
```

Each example program will be built under a subdirectory of the build folder.
For example, on Windows, the file `build/Example1/Example1.exe` will be built
if the build directory was named `build` as in the above example.
