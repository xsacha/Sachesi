#!/bin/bash

qmake -spec unsupported/blackberry-armv7le-g++ ..
make
./build-package.sh DEBUG
