#!/bin/bash

cd `dirname $0`

(mkdir -p build && cd build && cmake . && make)
afb-binder --config=hackaton.config -vvv
