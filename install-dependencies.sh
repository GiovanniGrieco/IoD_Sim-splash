#!/bin/bash

source /etc/os-release

if [[ "$id" -eq "debian" ]]; then
  echo "${NAME} has been detected in your system. Installing IoD_Sim required packages automatically."
  echo "Please insert your sudo password when requested to gain administration privileges."

  sudo apt install -y --no-install-recommends \
    cmake        \
    gcc          \
    clang        \
    jq           \
    llvm-dev     \
    libclang-dev \
    make

else
  echo "${NAME} is not supported by this script. Please, install IoD Sim dependencies manually by following the official guide."
  exit 1
fi
