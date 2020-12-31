#!/usr/bin/bash

clang -x c++ -I../IoD_Sim/Development/ns3/build/ -emit-ast -o model.pch ../IoD_Sim/Development/drone/model/constant-acceleration-drone-mobility-model.cc
