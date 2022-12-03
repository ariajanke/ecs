#!/bin/bash
sources="main.cpp HashTableEntity.cpp AvlTreeEntity.cpp"
includes="-I../lib/cul/inc -I../inc"
enablecoverage="-fprofile-instr-generate -fcoverage-mapping"
defaultflags="-std=c++17 -Wno-unqualified-std-cast-call -O3 -Wall -pedantic-errors -DMACRO_PLATFORM_LINUX -DMACRO_ARIAJANKE_ECS3_ENABLE_TYPESET_TESTS -fexceptions"
g++ $defaultflags $sources $includes -o .unit-tests
valgrind ./.unit-tests

