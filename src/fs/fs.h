#pragma once

#include <QFile>
#include <QIODevice>
#include <QDir>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

// node.mode flags
#define QCFM_IS_COMPRESSED      ((1 << 22) | (1 << 23) | (1 << 24))
// lzo1x_decompress_safe
#define QCFM_IS_LZO_COMPRESSED   (1 << 23)
// ucl_nrv2b_decompress_8
#define QCFM_IS_UCL_COMPRESSED  ((1 << 22) | (1 << 23))
#define QCFM_IS_GZIP_COMPRESSED  (1 << 22)
#define QCFM_IS_DIRECTORY        (1 << 14)
#define QCFM_IS_SYMLINK          (1 << 13)
// QNX6 maximum filename length
#define QNX6_MAX_CHARS 510

// Utility function to quickly grab from stream
#define READ_TMP(x, y) x y; stream >> y;
// Maximum amount to store in RAM at any time between reading QIODevice and writing to disk.
#define BUFFER_LEN (qint64)4096

#ifdef _WIN32
#include "Windows.h"
// We need to update the time of the extracted file based on the unix filesystem it comes from.
void fixFileTime(QString filename, int time);
#endif

// Since QNX files are always LittleEndian but Qt defaults to BigEndian, this wrapper exists
class QNXStream : public QDataStream {
public:
    QNXStream(QIODevice * device)
    : QDataStream(device) {
        setByteOrder(QDataStream::LittleEndian);
    }
    QNXStream(QByteArray * a, QIODevice::OpenMode flags)
    : QDataStream(a, flags) {
        setByteOrder(QDataStream::LittleEndian);
        resetStatus();
    }
};

class QFileSystem : public QObject
{
    Q_OBJECT
public:
    explicit QFileSystem(QString filename, QIODevice* file = nullptr, qint64 offset = 0, qint64 size = 0, QString path = ".", QString imageExt = "");
    ~QFileSystem();

    // Basis of size reporting
    void increaseCurSize(qint64 read) {
        if (read <= 0)
            return;
        curSize += read;
        emit sizeChanged(read);
    }

    QString uniqueDir(QString name);
    QString uniqueFile(QString name);
    virtual QString generateName(QString imageExt = "");
    bool writeFile(QString fileName, qint64 offset, qint64 writeSize, bool absolute = false);
    bool extractImage();
    virtual bool createImage(QString name);
    bool extractContents();
    virtual bool createContents() = 0;

    qint64 curSize;
    qint64 maxSize;

signals:
    void sizeChanged(qint64 delta);

protected:
    QIODevice* _file;
    qint64 _offset, _size;
    QString _path, _filename;
    QString _imageExt;
};
