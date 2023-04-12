#!/bin/bash

(mkdir -p build && cd build && cmake . && make)
export BUIDDIR=$(pwd)/build; afb-binder --config=hackaton.config -vvv
