#ifndef FIRSTRUN_H
#define FIRSTRUN_H

#include <QDir>

class FindInHomeDir : public QObject
{
    Q_OBJECT
    using BaseClass = QObject;

public:
    explicit    FindInHomeDir(QObject* parent = nullptr);
    ~FindInHomeDir();

    QString     FindFileInHomeDir(QString strFileName);

public slots:

signals:

private slots:

private:
    void        SearchInDir(const char *chrName, const char *chrFile,int iIndent);
    char        chrResultPath[2048];
    int         iTestFileDone;
};

#endif // FIRSTRUN_H
