# Sachesi

Introducing Sachesi. The results of my continued work on firmware tools for Blackberry 10 and Playbook.

Sachesi allows you to extract, search for and (un)install Blackberry firmware. It also allows you to backup, restore, wipe, reboot and nuke. This is a continued evolution of the original Sachup and Sachibar applications.
None of its activities require development mode. That is, you can sideload and uninstall applications without developer mode.

The application mimics communications performed by official Blackberry tools and allows modification of the typically fixed commands that are sent from the computer. This allows increased control and flexibility over firmware related activies on your device.

Developed by Sacha Refshauge. Project originally known as Dingleberry. Public release of source code on May 26, 2014.


## Build Instructions

Technically should work on all operating systems that support Qt. This project works with both dynamic and static builds of Qt4.8+ and Qt5.0+.
It is known to build and has built binaries available for desktop platforms: Windows XP+, Linux, Mac OSX 10.5+. It can also build and run a restricted subset of activities on mobile platforms: Symbian, Android and Blackberry 10. To upgrade firmware via a mobile device requires USB low-level access (host device must support network usb drivers), such as Symbian and possibly rooted Android devices.

This project requires miniLZO, zlib-1.2.8, QuaZIP and OpenSSL.
For Linux and Mac, the project optionally uses libusb-1.0 for bootloader activities.
For your convenience, a snapshot of the important files from libusb-1.0, miniLZO, zlib-1.2.8 and QuaZIP have been provided. OpenSSL for Android is also provided.


## Windows Build Instructions (MinGW)

1. Install latest QT with MinGW from [qt-project.org](https://qt-project.org).
2. Install OpenSSL to `C:\OpenSSL` from [http://www.wittfella.com/openssl](http://www.wittfella.com/openssl).
3. Open Sachesi.pro in QtCreator.
4. Adjust the hardcoded paths set in `QML.qrc` to point to the correct qml files.
5. Run.

**Notes for Visual Studio**:

- Use [this precompiled OpenSSL](http://www.npcglib.org/~stathis/blog/precompiled-openssl) version.


## Ubuntu Build Instructions

1. Install the required dependencies:

```bash
sudo apt-get install qt5-qmake libqt5network5 libqt5qml5 qtdeclarative5-settings-plugin qtdeclarative5-controls-plugin qtdeclarative5-dialogs-plugin qtdeclarative5-quicklayouts-plugin qtdeclarative5-window-plugin qtdeclarative5-qtquick2-plugin qttools5-dev-tools libssl-dev qtdeclarative5-dev libusb-1.0-0-dev
```

2. Compile Sachesi:

```bash
cd /path/to/Sachesi;
qmake;
make -j4;
```

**Common Errors**

On some systems there might be some `Cannot find file translations/...` errors. This can
be fixed by executing `make compiler_lang_make_all` and executing the above steps again.


## OSX Build Instructions

1. Install latest XCode.
2. Install latest QT from [qt-project.org](https://qt-project.org).
3. Install `brew` from [brew.sh](https://brew.sh).
4. Run `brew install libusb`.
5. Open Sachesi.pro in QtCreator.
6. Run.

