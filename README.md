# Splash

Splash is a transpiler from C++ to Python to extend [Airflow](https://github.com/GiovanniGrieco/IoD_Sim-airflow) Visual Programming Language environment with basic
tools to configure and create simulation scenarios on [IoD Sim](https://github.com/telematics-lab/IoD_Sim).

Splash inner workings are described along the entire simulation platform on [arXiv](https://arxiv.org/abs/2203.13710).

## Requirements
Splash requires the following software to be used:
* cxxopts
* boost-json
* libclang (from the llvm project)

## Quick Start on Linux-based OS without vcpkg
```bash
$ ./install-dependencies.sh
$ cmake -B build -S .
```

## Quick Start with vcpkg
If you use vcpkg, you can install those requirements via vcpkg. Here's some sample commands:
```powershell
# switch current working directory to vcpkg
> ./vcpkg install cxxopts
> ./vcpkg install boost-json
> ./vcpkg install llvm[clang]
# switch working directory to splash
> cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 16 2019" -A x64
```

## Compatibility
This project has been successfully tested on Linux and Windows.
