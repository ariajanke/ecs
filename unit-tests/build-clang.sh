#!/bin/bash
sources="main.cpp HashTableEntity.cpp AvlTreeEntity.cpp"
includes="-I../lib/cul/inc -I../inc"
enablecoverage="-fprofile-instr-generate -fcoverage-mapping"
defaultflags="-std=c++17 -O1 -Wall -pedantic-errors -DMACRO_PLATFORM_LINUX -DMACRO_ARIAJANKE_ECS3_ENABLE_TYPESET_TESTS"
clang++ $defaultflags $enablecoverage $sources $includes -o .unit-tests
./.unit-tests
#llvm-profdata merge -sparse default.profraw -o ut.profdata
#llvm-cov report -ignore-filename-regex='cpp|cul|shared' ./.unit-tests -instr-profile=ut.profdata
