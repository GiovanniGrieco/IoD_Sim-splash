#!/bin/bash

source /etc/os-release

function detected_message() {
  echo "${NAME} has been detected in your system. Installing IoD_Sim required packages automatically."
  echo "Please insert your sudo password when requested to gain administration privileges."
}

case "$ID" in
  debian | ubuntu)
    detected_message
    sudo apt install -y --no-install-recommends \
      cmake        \
      gcc          \
      clang        \
      jq           \
      llvm-dev     \
      libclang-dev \
      make
    ;;

  fedora | centos)
    detected_message
    sudo dnf install -y \
      boost-devel   \
      cmake         \
      cxxopts-devel \
      gcc           \
      clang         \
      jq            \
      llvm-devel    \
      clang-devel   \
      make
    ;;

  *)
    echo "${NAME} is not supported by this script. Please, install IoD Sim dependencies manually by following the official guide."
    exit 1
    ;;
esac
