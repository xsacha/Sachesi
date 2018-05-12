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

#include "installer.h"

void InstallNet::requestAuthenticate()
{
    logadd(QString("8. Request Authenticate"));
    QByteArray buffer;
    QDataStream config(&buffer, QIODevice::WriteOnly);
    config << qint16(6) /*messageSize*/ << qint16(2) /*version*/ << qint16(8) /*code*/;
    sock->write(buffer);
}

void InstallNet::AESEncryptSend(QByteArray &plain, int code)
{
    /* Encrypt Plain Text */
    unsigned char* iv = new unsigned char[16];
    RAND_bytes(iv, 16);
    int ilen, tlen;
    unsigned char* encrypt = new unsigned char[plain.length() + 16];
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);
    const EVP_CIPHER *cipher = EVP_aes_128_cbc();
    EVP_EncryptInit(ctx, cipher, (unsigned char*)sessionKey.data(), iv);
    EVP_EncryptUpdate(ctx, encrypt, &ilen, (unsigned char*)plain.data(), plain.length());
    EVP_EncryptFinal(ctx, encrypt + ilen, &tlen);
    EVP_CIPHER_CTX_free(ctx);

    /* Create buffer */
    QByteArray buffer;
    QDataStream Auth(&buffer, QIODevice::WriteOnly);
    Auth << qint16(ilen + tlen) << qint16(plain.length());
    Auth.writeRawData((char*)iv,16);
    Auth.writeRawData((char*)encrypt, ilen+tlen);

    /* Package and Send */
    QByteArray header;
    QDataStream FrameHead(&header, QIODevice::WriteOnly);
    FrameHead << qint16(6 + buffer.length()) << qint16(2) << qint16(code);
    buffer.prepend(header);
    sock->write(buffer);
    delete [] iv;
    delete [] encrypt;
}

void InstallNet::authorise()
{
    logadd(QString("10. Authorise"));
    /* Construct Plain Text */
    QByteArray plain(2+hashedPassword.length(),0);
    QDataStream plainText(&plain, QIODevice::WriteOnly);
    plainText << qint16(hashedPassword.length());
    plainText.writeRawData(hashedPassword.data(),hashedPassword.length());

    AESEncryptSend(plain, 10);
}

void InstallNet::keepAlive()
{
    logadd(QString("6. Keep Alive"));
    QByteArray buffer;
    QDataStream config(&buffer, QIODevice::WriteOnly);
    config << qint16(6) /*messageSize*/ << qint16(2) /*version*/ << qint16(6) /*code*/;
    sock->write(buffer);
}
