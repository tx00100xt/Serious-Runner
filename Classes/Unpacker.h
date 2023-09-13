#ifndef UNPACKER_H
#define UNPACKER_H

#include <QMessageBox>

class Unpacker : public QObject
{
    Q_OBJECT
    using BaseClass = QObject;

public:
    explicit Unpacker(QObject* parent = nullptr);
    ~Unpacker();

    void        Extract(QString strFileName, QString strPath, int iDoExtract, int iFlags);
    QString     strFile;

public slots:

signals:
    void        signalCurrentUnpackFile(const QString &strFileName);
    void        signalUnpackEnd();

private slots:

private:
    int         CopyData(struct archive *ar, struct archive *aw);
    void        MsgBox(int type, QString strMessage);
    QMessageBox msgBox;
};

#endif // UNPACKER_H
