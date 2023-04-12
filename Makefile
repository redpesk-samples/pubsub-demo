# Copyright (C) 2015-2023 IoT.bzh Company
# Author: José Bollo <jose.bollo@iot.bzh>
#
# SPDX-License-Identifier: LGPL-3.0-only

FLAGS = -g \
	-fPIC \
	-shared \
	$(shell pkg-config --cflags --libs afb-binding) \
	-Wl,--version-script=$(shell pkg-config --variable=version_script afb-binding)

CFLAGS += ${FLAGS}
CXXFLAGS += ${FLAGS} -fpermissive

.PHONY: all clean

tutos = tuto-1.so tuto-3.so
samples = hello4.so hello4pp.so

all: sender.so

clean:
	rm *.so 2>/dev/null || true

%.so: %.c
	$(CC) $(CFLAGS) -o $@ $<

%.so: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<
