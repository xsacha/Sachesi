QT += network gui core declarative script
win32: QT += sql xmlpatterns

TARGET="Sachesi"
ICON=sachesi-114.png
VERSION=1.2.3

# Global specific
win32:CONFIG(release, debug|release): CONFIG_DIR = $$join(OUT_PWD,,,/release)
else:win32:CONFIG(debug, debug|release): CONFIG_DIR = $$join(OUT_PWD,,,/debug)
else:CONFIG_DIR=$$OUT_PWD
OBJECTS_DIR = $$CONFIG_DIR/.obj/$$TARGET
MOC_DIR = $$CONFIG_DIR/.moc/$$TARGET
UI_DIR = $$CONFIG_DIR/.ui/$$TARGET
P = $$_PRO_FILE_PWD_
INCLUDEPATH += $$P/ext $$P/src

DEFINES += SACHESI_VERSION='\\"$$VERSION\\"'
exists($$P/.git): GIT_VERSION = '\\"$$system(git rev-list HEAD --count)-$$system(git describe --always)\\"'
!isEmpty(GIT_VERSION): DEFINES += SACHESI_GIT_VERSION=\"$$GIT_VERSION\"

greaterThan(QT_MAJOR_VERSION, 4) {
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
    DEFINES += NOMINMAX _CRT_SECURE_NO_WARNINGS
    contains(CONFIG,static) {
        DEFINES += STATIC STATIC_BUILD
        LIBS+= -LC:\\openssl\\lib\\VC\\static -llibeay32MT -lssleay32MT -lGDI32 -lAdvapi32
    } else {
        LIBS+= -LC:\\openssl\\lib -llibeay32
    }
    # Hardcoded lib folder for winsocks
    LIBS+= -L"C:\\Program Files (x86)\\Windows Kits\\8.1\\Lib\\winv6.3\\um\\x86" -lWSock32 -lUser32 -lCrypt32
}
blackberry {
    DEFINES += BLACKBERRY
    LIBS += -lbbcascadespickers -lbbsystem -lQtXml #-lcrypto
}
mac {
    INCLUDEPATH += /opt/local/include
    LIBS+= -lcrypto -lssl -lz -framework CoreFoundation -framework IOKit -lobjc /opt/local/lib/libusb-1.0.a
}
!win32:!mac:!blackberry: {
    android {
        LIBS += $$P/Android/libcrypto.so $$P/Android/libssl.so
        INCLUDEPATH += $$P/Android/include/
    } else {
        LIBS += -ldl -lz -ludev
        # These should be static for it to be fully portable
        LIBS += -lcrypto -lusb-1.0
    }
}


SOURCES += \
    src/main.cpp \
    src/mainnet.cpp \
    src/splitter.cpp \
    src/lzo.cpp \
    src/ports.cpp
HEADERS += \
    src/mainnet.h \
    src/splitter.h \
    src/lzo.h \
    src/ports.h

!blackberry {
    SOURCES += \
        src/install.cpp \
        src/install_auth.cpp \
        src/install_establish.cpp \
        src/install_qml.cpp \
        src/apps.cpp \
        src/droparea.cpp \
        src/backupinfo.cpp
    HEADERS += \
        src/install.h \
        src/droparea.h \
        src/apps.h \
        src/backupinfo.h
}

!win32:!blackberry:!android {
    DEFINES += BOOTLOADER_ACCESS
    SOURCES += src/boot.cpp
    HEADERS += src/boot.h
    # This also means libusb needs to be linked
}

DEFINES += QUAZIP_STATIC
SOURCES += $$P/ext/quazip/*.cpp $$P/ext/quazip/*.c
HEADERS += $$P/ext/quazip/*.h

RESOURCES += bbupdatescan.qrc
blackberry {
    RESOURCES += UI_bb10.qrc
} else {
    RESOURCES += UI_generic.qrc
}

OTHER_FILES += \
    qml/generic/mcc.js \
    qml/generic/*.qml \
    qml/bb10/*.qml \
    qml/generic/UI/*.qml \
    qml/bb10/UI/*.qml \
    Android/AndroidManifest.xml

ANDROID_PACKAGE_SOURCE_DIR = $$P/Android

# Useful for adjusting paths and setting icons
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()
