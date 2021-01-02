#!/usr/bin/bash

clang -x c++ \
      -I../IoD_Sim/Development/ns3/build/ \
      -emit-ast \
      -o model.pch \
      $1
