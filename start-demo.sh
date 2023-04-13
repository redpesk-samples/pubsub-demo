#!/bin/bash

cd `dirname $0`

(mkdir -p build && cd build && cmake .. && make && cd ..)

export BUILDDIR=$(pwd)/build;

afb-binder --config=hackaton.config -vvv
