#!/bin/bash

mkdir -p build && cd build && cmake . && make
afb-binder --config=hackaton.config -vvv
