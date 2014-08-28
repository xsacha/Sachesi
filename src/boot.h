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

#pragma once

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QThread>
#include <QStringList>
#include <libusb-1.0/libusb.h>

#define BLACKBERRY_VENDOR_ID 0x0FCA

#define BUFFER_SIZE 2048

#ifdef _WIN32
#pragma pack(push,1)
#endif
// message with type 00
typedef struct ControlMessageHeader {
    unsigned short type; // 0000
    unsigned short packetSize;
    unsigned char command;
    unsigned char mode;
    unsigned short packetId;
    // any data specific to the message is added after
}
#ifndef _WIN32
__attribute__((packed)) ControlMessageHeader;
#else
ControlMessageHeader;
#pragma pack(pop)
#endif

enum SachesiCommand {
    commandInfo = 0x01,
    commandBootloader = 0x02,
    commandNuke = 0x03,
    commandDebug = 0x04
};

enum ControlMessageCommand {
    kCommandPing                         = 0x01, // OUT
    kCommandPingResponse                 = 0x02, // IN
    kCommandReboot                       = 0x03, // OUT
    kCommandRebootResponse               = 0x04, // IN
    kCommandGetVariable                  = 0x05, // OUT
    kCommandGetVariableResponse          = 0x06, // IN
    kCommandSetMode                      = 0x07, // OUT
    kCommandSetModeSuccess               = 0x08, // IN
    kCommandSetModeFailure               = 0x09, // IN
    kCommandGetPasswordInfo              = 0x0A, // OUT
    kCommandLoadTransferredData          = 0x0B, // IN/OUT
    kCommandLoadTransferredDataSuccess   = 0x0C, // IN/OUT
    kCommandLoadTransferredDataFailure   = 0x0D, // IN/OUT
    kCommandGetPasswordInfoResponse	     = 0x0E, // IN
    kCommandSendPassword                 = 0x0F, // OUT
    kCommandSendPasswordCorrect          = 0x10, // IN
    kCommandUnkn1                        = 0x11, // ?
    kCommandUnkn1Response                = 0x12, // OUT
    kCommandReadyForDataTransfer         = 0x13 // IN
};

typedef enum BootloaderMode {
    // value = mode entry in message 00 struct when mode is set
    kBootloaderRimReinit                 = 0x0,
    kBootloaderRimBoot                   = 0x1,
    kBootloaderRamBoot                   = 0x2,
    kBootloaderUPL                       = 0x3,

    // Modes with no 'Mode Entry'
    kBootloaderRimBootNuke               = 0x4 // Wipe User partition
} BootloaderMode;

class Boot : public QThread
{
    Q_OBJECT
    Q_PROPERTY(bool        connecting READ connecting WRITE setConnecting NOTIFY connectingChanged)
    Q_PROPERTY(int         command READ command WRITE setCommand NOTIFY commandChanged)
    Q_PROPERTY(QStringList devices READ devices NOTIFY devicesChanged)
public:
    Boot();
    ~Boot();
    libusb_device_handle* openDevice(libusb_device* dev);
    int closeDevice(libusb_device_handle* handle);
    int sendControlMessage(libusb_device_handle* aHandle, uint8_t aCommand, QByteArray* aData, int aDataSize);
    int receiveControlMessage(libusb_device_handle* aHandle, ControlMessageHeader* aHeader, QByteArray* aData);
    void commandReboot(libusb_device_handle* aHandle);
    void commandPing(libusb_device_handle* aHandle);
    bool commandSendPass(libusb_device_handle* aHandle);
    void commandSendLoader(libusb_device_handle* aHandle);
    int commandGetVariable(libusb_device_handle* aHandle, int variable, QByteArray* buffer, int bufferSize);
    void commandSetMode(libusb_device_handle* aHandle, BootloaderMode mode);
    void commandLoadBootloader(libusb_device_handle* aHandle);
    void commandTransferData(libusb_device_handle* aHandle);

    bool connecting() const;
    int command() const;
    QStringList devices() const;
    void setCommand(const int &command);
    void setConnecting(const bool &connecting);
signals:
    void connectingChanged();
    void commandChanged();
    void devicesChanged();
public slots:
    Q_INVOKABLE void search();
    Q_INVOKABLE void connect();
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void setCommandMode(int mode, bool rebootAfter);
    void newPassword(QString newPass);
    void exit() {
        _kill = true;
    }
private:
    int _bootloaderMode;
    bool _connecting;
    int _command;
    bool _rebootAfter;
    short _packetNum;
    int _found;
    QString _password;
    QStringList _devices;
    QList<QByteArray*> bootloaderModeData;
    bool _kill;
};
