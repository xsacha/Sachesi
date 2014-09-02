// Copyright (C) 2014 Sacha Refshauge

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 3.0 for more details.

// A copy of the GPL 3.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official GIT repository and contact information can be found at
// http://github.com/xsacha/Sachesi

#include "ports.h"
#include "boot.h"
#include "install.h"
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QBuffer>
#include <QDataStream>

Boot::Boot() : _bootloaderMode(0xFF), _connecting(false), _command(0), _rebootAfter(false), _packetNum(0), _kill(false)
{
    bootloaderModeData.append(new QByteArray("RIM REINIT\0\0\0\0\0\0\1",    17));
    bootloaderModeData.append(new QByteArray("RIM-BootLoader\0\0\1",17));
    bootloaderModeData.append(new QByteArray("RIM-RAMLoader\0\0\0\1", 17));
    bootloaderModeData.append(new QByteArray("RIM UPL\0\0\0\0\0\0\0\0\0\1",       17));
    bootloaderModeData.append(new QByteArray("RIM-BootNUKE\0\0\0\0\1",  17));

    // If Android does not have USB permissions, this will fail and we need to handle it.
    _kill = libusb_init(nullptr) != 0;
    // Should probably just shut down and notify qml if _kill is false at this stage.
}

Boot::~Boot()
{
    libusb_exit(nullptr);
}

libusb_device_handle* Boot::openDevice(libusb_device* dev) {
    int configuration = 0;
    libusb_device_handle* handle = nullptr;

    libusb_open(dev, &handle);
    if (handle == nullptr) {
#ifdef _WIN32
        QMessageBox::information(nullptr, "Error", "You need to install WinUSB driver for the Blackberry device.");
#else
        QMessageBox::information(nullptr, "Error", "You need to run this application with root privileges (i.e. sudo).");
#endif
        libusb_close(handle);
        return nullptr;
    }

    libusb_get_configuration(handle, &configuration);

    libusb_detach_kernel_driver(handle, 1);
    if(configuration != 1) {
        if (libusb_set_configuration(handle, 1) < 0) {
            QMessageBox::information(nullptr, "Error", "Error #1001");
            return nullptr;
        }
    }

    libusb_detach_kernel_driver(handle, 0);
    int err = libusb_claim_interface(handle, 0);
    if (err < 0) {
#ifdef Q_OS_MAC
        QMessageBox::information(nullptr, "Error", "Please reboot your device manually.");
        return nullptr;
#else
        QMessageBox::information(nullptr, "Error", "Error #1002 (" + QString::number(err) + ")");
        return nullptr;
#endif
    }
    return handle;
}

int Boot::closeDevice(libusb_device_handle* handle) {
    if (handle == nullptr) {
        return -1;
    }

    libusb_release_interface(handle, 0);
    libusb_release_interface(handle, 1);
    libusb_close(handle);
    return 0;
}

int Boot::sendControlMessage(libusb_device_handle* aHandle, uint8_t aCommand, QByteArray* aData, int aDataSize) {
    QByteArray buffer(aDataSize + 8, 0);
    ControlMessageHeader* header = (ControlMessageHeader*)buffer.data();
    header->type = 0x00;
    header->packetSize = aDataSize + 8;
    header->command = aCommand;
    header->mode = _bootloaderMode;
    header->packetId = _packetNum++;
    if (aData)
        memcpy((unsigned char*)buffer.data()+8, (const unsigned char*)aData->constData(), aDataSize);

    int transferred = 0;
    int err = libusb_bulk_transfer(aHandle, _found == 0x1 ? 0x2 : 0x1, (unsigned char*)buffer.data(), aDataSize + 8, &transferred, 1000);
    if (err == -7)
    {
        QMessageBox::information(nullptr, "Error", "Was not able to send message to connected device.");
        return -7;
    }

    return transferred;
}

int Boot::receiveControlMessage(libusb_device_handle* aHandle, ControlMessageHeader* aHeader, QByteArray* aData) {
    QByteArray buffer(BUFFER_SIZE, 0);
    int transferred = 0;
    int ret = libusb_bulk_transfer(aHandle, _found == 0x1 ? 0x82 : 0x81, (unsigned char*)buffer.data(), BUFFER_SIZE, &transferred, 1000);
    if (ret == -7) {
        if (_found != 0x1) {
#ifdef _WIN32
            QMessageBox::information(nullptr, "Error", "Make sure the device is in 'Windows' mode.");
#else
            QMessageBox::information(nullptr, "Error", "Make sure the device is in 'Mac' mode.");
#endif
        } else {
            QMessageBox::information(nullptr, "Error", "The connected device did not respond as expected.");
        }
        return -7;
    }

    if (transferred < 8)
        return 0;

    /*qDebug() << "Header: " << buffer.mid(4,4).toHex();
    if (transferred > 8) {
        if ((transferred - 8) < 40)
            qDebug() << "Text: " << buffer.mid(8,transferred - 8);
        qDebug() << "Hex: " << buffer.mid(8,transferred - 8).toHex();
    }*/


    if (aHeader)
        memcpy(aHeader, (const unsigned char*)buffer.constData(), 8);
    if (aData)
        memcpy((unsigned char*)aData->data(), (const unsigned char*)buffer.constData() + 8, transferred - 8);

    return transferred;
}

void Boot::commandReboot(libusb_device_handle* aHandle) {
    sendControlMessage(aHandle, kCommandReboot, nullptr, 0); // reboots the device (message 00 command 03)
    receiveControlMessage(aHandle, nullptr, nullptr); // discard the reboot confirmation message (message 00 command 04)
}

void Boot::commandPing(libusb_device_handle* aHandle) {
    sendControlMessage(aHandle, kCommandPing, nullptr, 0); // reboots the device (message 00 command 03)
    receiveControlMessage(aHandle, nullptr, nullptr); // discard the reboot confirmation message (message 00 command 04)
}

int Boot::commandGetVariable(libusb_device_handle* aHandle, int variable, QByteArray* buffer, int bufferSize) {
    // gets a variable from the device (message 00 command 05)
    // returns the amount of data received
    // 0-1: buffer size
    // 2-3: variable
    uint16_t data[2];
    data[0] = bufferSize;
    data[1] = variable;
    QByteArray newBuf((const char*)data, 4);
    int err = sendControlMessage(aHandle, kCommandGetVariable, &newBuf, 4);
    if (err == -7)
        return -7;

    err = receiveControlMessage(aHandle, nullptr, buffer);
    if (err == -7)
        return -7;


    return err - 8;
}

void Boot::commandSetMode(libusb_device_handle* aHandle, BootloaderMode mode) {
    sendControlMessage(aHandle, kCommandSetMode, bootloaderModeData.at(mode), 16); // set the bootloader mode on the device (message 00 command 07)

    // update the bootloader mode
    ControlMessageHeader header;
    receiveControlMessage(aHandle, &header, nullptr);
    _bootloaderMode = header.mode;
}

void Boot::commandLoadBootloader(libusb_device_handle* aHandle) {
    sendControlMessage(aHandle, kCommandLoadTransferredData, nullptr, 0); // loads the usb bootloader or reboots (message 00 command 0B) // needs to be in mode 01 (RIM-BootLoader) to work
    receiveControlMessage(aHandle, nullptr, nullptr); // discard the mode confirmation message (message 00 command 0C)
}

void Boot::commandTransferData(libusb_device_handle* aHandle) {
    sendControlMessage(aHandle, kCommandReadyForDataTransfer, nullptr, 0); // loads the usb bootloader or reboots (message 00 command 0B) // needs to be in mode 01 (RIM-BootLoader) to work
    receiveControlMessage(aHandle, nullptr, nullptr);
}

bool Boot::commandSendPass(libusb_device_handle* aHandle) {
    sendControlMessage(aHandle, kCommandGetPasswordInfo, nullptr, 0); // get the device password info (message 00 command 0A)  // needs to be in mode 01 (RIM-BootLoader) to work
    QByteArray challengeData(100,0);
    receiveControlMessage(aHandle, nullptr, &challengeData); // discard the mode confirmation message (message 00 command 0E)

    QByteArray challenge = challengeData.mid(4, 4);
    QByteArray salt = challengeData.mid(12,8);
    QByteArray countData = challengeData.mid(20,4);
    QDataStream stream(countData);
    stream.setByteOrder(QDataStream::LittleEndian);
    int iterations;
    stream >> iterations;
    QByteArray hashedData = QByteArray(_password.toLatin1());
    int count = 0;
    bool challenger = true;
    do {
        QByteArray buf(4 + salt.length() + hashedData.length(),0);
        QDataStream buffer(&buf,QIODevice::WriteOnly);
        buffer.setByteOrder(QDataStream::LittleEndian);
        buffer << qint32(count);
        buffer.writeRawData(salt, salt.length());
        buffer.writeRawData(hashedData, hashedData.length());
        if (!count) hashedData.resize(64);
        SHA512((const unsigned char*)buf.data(), buf.length(), (unsigned char *)hashedData.data());
        if ((count == iterations - 1) && challenger)
        {
            count = -1;
            challenger = false;
            hashedData.prepend(challenge);
        }
    } while (++count < iterations);
    hashedData.prepend(QByteArray::fromHex("00004000"),4);

    sendControlMessage(aHandle, kCommandSendPassword, &hashedData, 68);
    ControlMessageHeader header;
    receiveControlMessage(aHandle, &header, nullptr);
    if (header.command == 0x10)
        return true;
    else {
        QMessageBox::information(nullptr,"Wrong Password.","You entered the wrong password. I don't handle this yet but basically you will need to go to the Install tab, fix your password, then try again.");
        return false;
    }
}

void Boot::commandSendLoader(libusb_device_handle *aHandle)
{
    // Should be telling us it's ready.
    ControlMessageHeader header;
    receiveControlMessage(aHandle, &header, nullptr);
    if (header.command != kCommandReadyForDataTransfer)
        return;
    int transferred = 0;
    unsigned char startSend[] = {0x1, 0x0, 0x10, 0x0, 0x7B, 0x9, 0x2B, 0x96, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xF0};
    libusb_bulk_transfer(aHandle, 0x2, startSend, 16, &transferred, 1000);
    receiveControlMessage(aHandle, &header, nullptr); // Should give Info basically
}

void Boot::search() {
    if (_kill)
        return;

    libusb_device_handle* handle = nullptr;
    libusb_device *dev = nullptr;
    libusb_device **list;
    libusb_get_device_list(nullptr, &list);
    _found = 0;
    struct libusb_device_descriptor desc;
    _devices.clear();
    for (int i = 0; (dev = list[i]) != nullptr; i++) {
        libusb_get_device_descriptor(dev, &desc);
        if (desc.idVendor == BLACKBERRY_VENDOR_ID) {
            _devices.append(QString::number(desc.idProduct,16));
            if (connecting()) {
                _found = desc.idProduct;
                handle = openDevice(list[i]);
                // We need to reboot it to talk to it
                if (desc.idProduct != 0x1) {
                    if (handle != nullptr) {
                        commandReboot(handle);
                        closeDevice(handle);
                    } else
                        _found = 0;
                }
            }
            break;
        }
    }
    libusb_free_device_list(list, 1);
    emit devicesChanged();
    if (_found != 0x1) {
        // We'll be rebooting this guy
        QTimer::singleShot(10000, this, SLOT(search()));
        return;
    }
    QTimer::singleShot(1500, this, SLOT(search()));
    if (handle == nullptr) {
        if (_found == 0x1)
            setConnecting(false);
        return;
    }
    setConnecting(false);

    switch(command()) {
    case commandInfo: {
        commandSetMode(handle, kBootloaderRimBoot);
        QByteArray buffer(2000, 0);
        int received = commandGetVariable(handle, 2, &buffer, 2000);
        if (received == -7)
            return;
        QFile info(getSaveDir() + "info.txt");
        info.open(QIODevice::WriteOnly);
        QBuffer bufferStream(&buffer);
        bufferStream.open(QIODevice::ReadOnly);
        QDataStream stream(&bufferStream);
        stream.setByteOrder(QDataStream::LittleEndian);
        short messageSize;
        stream >> messageSize;
        info.write(QByteArray("Message Size: ") + QByteArray::number(messageSize) + "\n");
        bufferStream.seek(16);
        unsigned int pin;
        stream >> pin;
        info.write(QByteArray("Hardware ID: 0x") + QByteArray::number(pin, 16) + " " + bufferStream.read(64).split('\0').first() + "\n");
        info.write(QByteArray("Build User: ") + bufferStream.read(16).split('\0').first() + "\n");
        info.write(QByteArray("Build Date: ") + bufferStream.read(16).split('\0').first() + "\n");
        info.write(QByteArray("Build Time: ") + bufferStream.read(16).split('\0').first() + "\n");
        unsigned int unk1;
        stream >> unk1;
        info.write(QByteArray("Unknown value: 0x" + QByteArray::number(unk1, 16)) + "\n");
        bufferStream.seek(188);
        unsigned int baseId, brId;
        stream >> baseId;
        stream >> brId;
        info.write(QByteArray("Hardware OS ID: 0x" + QByteArray::number(baseId, 16)) + "\n");
        info.write(QByteArray("BR ID: 0x" + QByteArray::number(brId, 16)) + "\n");
        info.write(buffer.mid(bufferStream.pos(), received - bufferStream.pos()));
        bufferStream.close();
        openFile(info.fileName());
        info.close();
    }
        break;
    case commandBootloader:
        commandSetMode(handle, kBootloaderRimBoot);
        commandSendPass(handle);
        if (commandSendPass(handle))
            commandSendLoader(handle);
        break;
    case commandNuke:
        commandSetMode(handle, kBootloaderRimBoot);
        commandSetMode(handle, kBootloaderRimBootNuke);
        break;
    case commandDebug:
        _found = 8000;
        commandSetMode(handle, kBootloaderRimBoot);
        break;
    default:
        commandSetMode(handle, kBootloaderRimBoot);
        break;
    }
    if (_rebootAfter)
        commandReboot(handle);

    closeDevice(handle);
}

void Boot::connect() {
    setConnecting(true);
}

void Boot::disconnect() {
    setConnecting(false);
}

void Boot::newPassword(QString newPass) {
    _password = newPass;
}

void Boot::setCommandMode(int mode, bool rebootAfter) {
    setCommand(mode);
    connect();
    _rebootAfter = rebootAfter;
}

#define SET_QML2(type, name, caps) \
    type Boot::name() const { \
    return _ ## name; \
    } \
    void Boot::caps(const type &var) { \
    if (var != _ ## name) { \
    _ ## name = var; \
    emit name ## Changed(); \
    } \
    }

SET_QML2(bool, connecting, setConnecting)
SET_QML2(int,  command, setCommand)
