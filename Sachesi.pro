QT += network
TARGET="Sachesi"
ICON=sachesi-114.png

P = $$_PRO_FILE_PWD_
INCLUDEPATH += $$P/ext

greaterThan(QT_MAJOR_VERSION,4) {
    win32 {
        DEFINES += Q_WS_WIN32
        SOURCES += $$P/ext/zlib-win/*.c
        HEADERS += $$P/ext/zlib-win/*.h
        INCLUDEPATH += $$P/ext/zlib-win
    }
}

win32 {
    # Where is your OpenSSL Install? Hardcoded for Win32
    INCLUDEPATH += C:\\openssl\\include

    # Is static?
    CONFIG += static
    DEFINES += QUAZIP_STATIC NOMINMAX _CRT_SECURE_NO_WARNINGS
    contains(CONFIG,static) {
        DEFINES += STATIC STATIC_BUILD
        LIBS+= -LC:\\openssl\\lib\\VC\\static -llibeay32MT -lssleay32MT -lGDI32 -lAdvapi32
    } else {
        LIBS+= -LC:\\openssl\\lib -llibeay32
    }
    # Hardcoded lib folder for winsocks
    LIBS+= -L"C:\\Program Files (x86)\\Windows Kits\\8.1\\Lib\\winv6.3\\um\\x86" -lWSock32 -lUser32 -lCrypt32
}
blackberry: LIBS += -lbbcascadespickers -lbbsystem -lQtXml -L$$P/Blackberry -lquazip #-lcrypto
mac {
    INCLUDEPATH += /opt/local/include
    LIBS+= -lcrypto -lssl -lz -framework CoreFoundation -framework IOKit -lobjc $$P/libquazip.a /opt/local/lib/libusb-1.0.a
}
!win32:!mac:!blackberry: {
    android {
        DEFINES += QUAZIP_STATIC
        LIBS += $$P/Android/libcrypto.so $$P/Android/libssl.so
        INCLUDEPATH += $$P/Android/include/
    } else {
        LIBS+= /usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libquazip.a /usr/lib/x86_64-linux-gnu/libusb-1.0.a -ldl -lz -ludev
    }
}


SOURCES += main.cpp \
    mainnet.cpp \
    splitter.cpp \
    lzo.cpp
!blackberry: SOURCES += \
    install.cpp \
    install_auth.cpp \
    install_establish.cpp \
    install_qml.cpp \
    apps.cpp \
    droparea.cpp \
    backupinfo.cpp

!win32:!blackberry:!android: SOURCES += boot.cpp

INCLUDEPATH += .

win32|android {
    SOURCES += $$P/ext/quazip/*.cpp $$P/ext/quazip/*.c
    HEADERS += $$P/ext/quazip/*.h
#    INCLUDEPATH += $$P/ext/quazip
}

blackberry: DEFINES += BLACKBERRY

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

QT += gui core declarative script
win32: QT += sql xmlpatterns

HEADERS += \
    mainnet.h \
    splitter.h \
    lzo.h
!blackberry: HEADERS += \
    install.h \
    droparea.h \
    apps.h \
    backupinfo.h
!win32:!blackberry:!android: HEADERS += boot.h

RESOURCES += bbupdatescan.qrc
blackberry {
        RESOURCES += UI_bb10.qrc
} else {
    win32 {
        RESOURCES += UI_win32.qrc
    } else {
        RESOURCES += UI_generic.qrc
    }
}

OTHER_FILES += \
    qml/generic/mcc.js \
    qml/generic/*.qml \
    qml/win32/*.qml \
    qml/bb10/*.qml \
    qml/generic/UI/*.qml \
    qml/win32/UI/*.qml \
    qml/bb10/UI/*.qml \
    Android/AndroidManifest.xml

ANDROID_PACKAGE_SOURCE_DIR = $$P/Android
