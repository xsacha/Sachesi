#!/bin/bash
# Usage: ./build-package.sh [DEBUG]

cp ../Sachesi .

if [ "$1" == "DEBUG" ]; then
    DEBUG="-devMode -debugToken /home/sacha/debugtoken.bar"
else
    DEBUG=""
fi

if [ -f ${QNX_TARGET}/usr/include/bps/vibration.h ]; then
    BB_OS=10.0
    QT_INSTALL_PATH=${QNX_TARGET}/armle-v7/usr/lib/qt4/
else
    echo "Could not find your Blackberry NDK. Please source bbndk-env.sh"
    exit 1
fi
echo "Building for Blackberry ${BB_OS}"

#
# Run the packaging utility
#
# -devMode -debugToken ~/debugToken.bar \
 
blackberry-nativepackager \
    -package Sachesi.bar \
    blackberry-tablet.xml Sachesi $DEBUG \
    -e ../sachesi-114.png res/sachesi-114.png

