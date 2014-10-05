QT += network gui widgets quick qml

TARGET="Sachesi"
TRANSLATIONS = translations/global.ts\
translations/ru.ts
win32: RC_ICONS += assets/sachesi-114.ico
else:mac: ICON = assets/sachesi-114.icns
else: ICON = assets/sachesi-114.png
VERSION = 1.9.2

# Global specific
CONFIG += c++11
INCLUDEPATH += ext src
P = $$_PRO_FILE_PWD_

win32 {
    SOURCES += $$P/ext/zlib-win/*.c
    HEADERS += $$P/ext/zlib-win/*.h
    INCLUDEPATH += ext/zlib-win
}

win32 {
    # Where is your OpenSSL Install? Hardcoded for Win32
    OPENSSL_PATH = C:\\OpenSSL
    INCLUDEPATH += $$OPENSSL_PATH\\include

    # Is all-in-one binary?
    CONFIG += static
    static: DEFINES += STATIC STATIC_BUILD

    !contains(QT_CONFIG, openssl-linked) {
        mingw: LIBS += -L$$OPENSSL_PATH -llibssl -llibcrypto -lgdi32
        else:static: LIBS += -L$$OPENSSL_PATH\\lib -llibeay32MT -lssleay32MT -lGDI32 -lAdvapi32
        else: LIBS += -L$$OPENSSL_PATH\\lib -llibeay32MT -lGDI32
    }

    !mingw {
        DEFINES += NOMINMAX _CRT_SECURE_NO_WARNINGS
        # Hardcoded lib folder for winsocks
        LIBS+= -L"C:\\Program Files (x86)\\Windows Kits\\8.1\\Lib\\winv6.3\\um\\x86" -lWSock32 -lUser32 -lCrypt32
    }
}
else:blackberry {
    DEFINES += BLACKBERRY
    LIBS += -lz #-lcrypto
}
else:mac {
    INCLUDEPATH += /opt/local/include
    LIBS+= -lcrypto -lssl -lz -framework CoreFoundation -framework IOKit -lobjc /opt/local/lib/libusb-1.0.a
    DEFINES += BOOTLOADER_ACCESS
}
else:freebsd-*|openbsd-* {
    isEmpty(PREFIX): PREFIX = /usr/local/
    LIBS += -lz -lcrypto -lusb
}
else:android {
    LIBS += -L $$P/Android -lcrypto -lssl -lusb1.0
    INCLUDEPATH += $$P/Android/include/
    ANDROID_EXTRA_LIBS += $$P/Android/libusb1.0.so
    ANDROID_PACKAGE_SOURCE_DIR = $$P/Android
    DEFINES += BOOTLOADER_ACCESS
} else {
    shared_quazip: LIBS += -lquazip
    shared_lzo2 {
        LIBS += -llzo2
        DEFINES += _LZO2_SHARED
    }
    LIBS += -lz -ldl -ludev
    # These below should be static for it to be fully portable (changing ABIs)
    LIBS += -lcrypto -lusb-1.0
    DEFINES += BOOTLOADER_ACCESS
}

SOURCES += \
    src/sachesi.cpp \
    src/mainnet.cpp \
    src/splitter.cpp \
    src/ports.cpp \
    src/apps.cpp \
    src/fs/ifs.cpp \
    src/fs/fs.cpp \
    src/fs/rcfs.cpp \
    src/fs/qnx6.cpp \
    src/appworldapps.cpp

HEADERS += \
    src/mainnet.h \
    src/splitter.h \
    src/ports.h \
    src/downloadinfo.h \
    src/apps.h \
    src/fs/ifs.h \
    src/fs/fs.h \
    src/fs/rcfs.h \
    src/fs/qnx6.h \
    src/carrierinfo.h \
    src/appworld.h \
    src/appworldapps.h

# Welcome to the only OS that won't give network access to USB device
!blackberry {
    SOURCES += \
        src/installer.cpp \
        src/installer_qml.cpp \
        src/installer_establish.cpp \
        src/installer_auth.cpp \
        src/backupinfo.cpp
    HEADERS += \
        src/installer.h \
        src/backupinfo.h
}

# This requires libusb to be linked
contains(DEFINES, BOOTLOADER_ACCESS) {
    SOURCES += src/boot.cpp
    HEADERS += src/boot.h
}

!shared_quazip {
    DEFINES += QUAZIP_STATIC
    include(ext/quazip/quazip.pri)
}
!shared_lzo2 {
    SOURCES += src/lzo.cpp
    HEADERS += src/lzo.h
}

RESOURCES += UI.qrc
# The qmldir is built in for dynamic libs but not static.
static: RESOURCES += QML.qrc
OTHER_FILES += \
    qml/generic/*.qml \
    qml/generic/UI/*.qml \
    Android/AndroidManifest.xml

# Qt Workaround for having install.cpp file
phony.depends = install uninstall
phony.target = .PHONY
QMAKE_EXTRA_TARGETS += phony

lupdate_only{
SOURCES = \
    qml/generic/mcc.js \
    qml/generic/*.qml \
    qml/generic/UI/*.qml
}
