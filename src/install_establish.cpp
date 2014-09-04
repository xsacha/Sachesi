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

#include "install.h"

void InstallNet::requestConfigure()
{
    if (!_state)
        return;
    logadd(QString("1. Request Configuration"));
    QByteArray buffer;
    QDataStream config(&buffer, QIODevice::WriteOnly);
    config << qint16(6) /*messageSize*/ << qint16(2) /*version*/ << qint16(1) /*code*/;
    sock->write(buffer);
}

void InstallNet::requestChallenge()
{
    logadd(QString("3. Request Challenge"));
    BIGNUM* e = BN_new(); BN_set_word(e, 65537);
    privkey = RSA_new();
    RSA_generate_key_ex(privkey, 1024, e, nullptr);
    unsigned char* privkey_mod = new unsigned char[BN_num_bytes(privkey->n)];
    BN_bn2bin(privkey->n, privkey_mod);
    QByteArray buffer(128,0);
    buffer = QByteArray::fromRawData((char*)privkey_mod,128);
    QByteArray header;
    QDataStream FrameHead(&header, QIODevice::WriteOnly);
    FrameHead << qint16(8 + buffer.length()) << qint16(2) << qint16(3) << qint16(buffer.length());
    buffer.prepend(header);
    sock->write(buffer);
    delete privkey_mod;
}

void InstallNet::replyChallenge()
{
    logadd(QString("5. Reply Challenge"));
    const signed char QCONNDOOR_PERMISSIONS[] = {3, 4, 118, -125, 1};
    const char EMSA_SHA1_HASH[] = {48, 33, 48, 9, 6, 5, 43, 14, 3, 2, 26, 5, 0, 4, 20};
    QCryptographicHash sha1(QCryptographicHash::Sha1);

    /* Encrypt challenge */
    QByteArray challengeBuffer = QByteArray((char*)serverChallenge, 30).append((const char*)QCONNDOOR_PERMISSIONS, 5);
    sha1.addData(challengeBuffer);
    QByteArray hash = QByteArray(EMSA_SHA1_HASH, 15).append(sha1.result());
    QByteArray signature(128, 0);
    RSA_private_encrypt(hash.length(),(unsigned char*)hash.data(), (unsigned char*)signature.data(), privkey, RSA_PKCS1_PADDING);

    /* Find Session Key */
    sessionKey = challengeBuffer.mid(8,16);

    /* Construct Plain Text */
    QByteArray plain(12 + challengeBuffer.length() + signature.length(),0);
    QDataStream plainText(&plain, QIODevice::WriteOnly);
    plainText << qint16(4 + challengeBuffer.length() + signature.length()) << qint16(challengeBuffer.length()) << qint16(signature.length());
    plainText.writeRawData(challengeBuffer.data(),35);
    plainText.writeRawData(signature.data(),128);

    AESEncryptSend(plain, 5);
}
