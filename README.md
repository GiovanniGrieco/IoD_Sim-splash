# Splash

Splash is a transpiler from C++ to Python to extend Visual Programming Language environment with basic
tools to configure and create simulation scenarios on IoD Sim. It was created during Formal Languages and Compilers
course by Sara Galasso and Giovanni Grieco.

## Requirements
Splash requires the following software to be used:
* cxxopts
* boost-json
* libclang (from the llvm project)

## Quick Start with vcpkg
If you use vcpkg, you can install those requirements via vcpkg. Here's some sample commands:
```powershell
# switch workring directory to vcpkg
> ./vcpkg install cxxopts
> ./vcpkg install boost-json
> ./vcpkg install llvm[clang]
# switch working directory to splash
> cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 16 2019" -A x64
```

## Compatibility
This project has been successfully tested on Linux and Windows.
