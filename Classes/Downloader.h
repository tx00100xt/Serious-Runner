#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QMessageBox>
#include <QUrl>
#include <QNetworkAccessManager>
#include "Classes/Unpacker.h"
#include "Classes/CopyDir.h"

class QNetworkReply;
class QFile;

class Downloader : public QObject
{
    Q_OBJECT
    using       BaseClass = QObject;

public:
    explicit    Downloader(QObject* parent = nullptr);
    ~Downloader();

    // Method to start the download
    bool        GetDB(const QString &strTargetFolder, const QUrl &urlLevel);
    bool        GetLevel(const QString &strTargetFolder, const QUrl &urlLevel);
    bool        GetMod(const QString &strTargetFolder, const QString &strTargetGame, const QUrl &urlModBin, const QUrl &urlModData);
    bool        GetGameSettings(const QString &strTargetFolder, const QUrl &urlGameBin, const QUrl &urlGameSettings);
    bool        GetMain(const QString &strTargetFolder, const QUrl &urlDownload, bool unpack);
    // Locked flag
    bool        locked_action = false;
    bool        locked_cancel = false;
    // Used classes
    Unpacker    m_unpack;
    CopyDir     m_copydir;

public slots:
    // Slots
    void        slotCancelDownload();
    void        slotCurrentUnpackFile(QString strCurrentFile);
    void        slotUnpackEnd();
    void        slotCopyDir(QString strCurrentFile);
    void        slotCopyDirEnd();

signals:
    // Signals
    void        signalUpdateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void        signalDownloadProgressEnd();
    void        signalCurrentUnpackFile(const QString &_text);
    void        signalUnpackEnd();
    void        signalCopyDir(const QString &_text);
    void        signalCopyDirEnd();
    void        signalTestGameInstall();
    void        signalSetDownloadLabel(const QString &_text);

private slots:
    // Slot for gradual reading of downloaded data
    void        slotOnReadyRead();
    // Slot to handle request completion
    void        slotOnReply(QNetworkReply* netReply);

private:
    // Private
    QNetworkReply*          m_currentReply  {nullptr};      // Current request being processed
    QFile*                  m_file          {nullptr};      // The current file being written to
    QNetworkAccessManager   m_manager;                      // Network manager for downloading files
    QMessageBox             msgBox;
    QString                 strMSG;
    QString                 strSecondTargetFolder = "";
    QUrl                    strSecondTargetUrl;
    QString                 strFirstTargetFolder = "";
    QUrl                    strFirstTargetUrl;
    // flags and pass count
    bool                    bModUnpack = false;
    bool                    bGameUnpack = false;
    int                     iPassCount = 0;
};

#endif // DOWNLOADER_H
