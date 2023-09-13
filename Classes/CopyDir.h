#ifndef COPYDIR_H
#define COPYDIR_H

#include <QFile>

class QFile;

class CopyDir : public QObject
{
    Q_OBJECT
    using BaseClass = QObject;

public:
    explicit    CopyDir(QObject* parent = nullptr);
    ~CopyDir();

    void        CopyAndReplaceFolderContents(const QString &strFromDir, const QString &strToDir, bool bCopyAndRemove);

public slots:

signals:
    // Signal transmitting information about the progress of the download
    void        updateCopyDirProgress(qint64 bytesReceived, qint64 bytesTotal);
    void        signalCopyDir(const QString &strFileName);
    void        signalCopyDirEnd();

private slots:

private:
    // Private
    bool        CopyPath(QString strSourceDir, QString strDestinationDir, bool bOverWriteDirectory);
};

#endif // COPYDIR_H
