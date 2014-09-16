#!/bin/bash
# Usage: ./build-package.sh [DEBUG]

if [ "$1" == "DEBUG" ]; then
    DEBUG="-devMode -debugToken /home/sacha/debugtoken.bar"
else
    DEBUG=""
fi

if [ -f ${QNX_TARGET}/usr/include/bps/vibration.h ]; then
    BB_OS=10.0
    QT_INSTALL_PATH=${QNX_TARGET}/armle-v7/usr/lib/qt5/lib/
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
    bar-descriptor.xml Sachesi $DEBUG \
    -e ../assets/sachesi-114.png res/sachesi-114.png \
    -e ${QT_INSTALL_PATH}/libQt5Core.so.5 lib/libQt5Core.so.5 \
    -e ${QT_INSTALL_PATH}/libQt5Gui.so.5 lib/libQt5Gui.so.5 \
    -e ${QT_INSTALL_PATH}/libQt5Widgets.so.5 lib/libQt5Widgets.so.5 \
    -e ${QT_INSTALL_PATH}/libQt5Network.so.5 lib/libQt5Network.so.5 \
    -e ${QT_INSTALL_PATH}/libQt5Qml.so.5 lib/libQt5Qml.so.5 \
    -e ${QT_INSTALL_PATH}/libQt5Quick.so.5 lib/libQt5Quick.so.5

