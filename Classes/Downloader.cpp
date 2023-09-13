#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "Classes/Downloader.h"

Downloader::Downloader(QObject* parent) :
    BaseClass(parent)
{
    // Connecting to the signals
    connect(&m_manager, &QNetworkAccessManager::finished, this, &Downloader::slotOnReply);
    connect(&m_unpack, &Unpacker::signalCurrentUnpackFile, this, &Downloader::slotCurrentUnpackFile);
    connect(&m_unpack, &Unpacker::signalUnpackEnd, this, &Downloader::slotUnpackEnd);
    connect(&m_copydir, &CopyDir::signalCopyDir, this, &Downloader::slotCopyDir);
    connect(&m_copydir, &CopyDir::signalCopyDirEnd, this, &Downloader::slotCopyDirEnd);
}

Downloader::~Downloader()
{

}

// Slots for signals
void Downloader::slotCurrentUnpackFile(QString strCurrentFile)
{
    emit signalCurrentUnpackFile(strCurrentFile);
}

void Downloader::slotUnpackEnd()
{
    emit signalUnpackEnd();
}

void Downloader::slotCopyDir(QString strCurrentFile)
{
    emit signalCopyDir(strCurrentFile);
}

void Downloader::slotCopyDirEnd()
{
    emit signalCopyDirEnd();
}

// Input functions
bool Downloader::GetDB(const QString &strTargetFolder, const QUrl &urlLevel)
{
    // Set type of download
    bModUnpack = false;
    bGameUnpack = false;
    iPassCount = 0;
    // Download
    bool bResult = GetMain( strTargetFolder, urlLevel, false);
    return bResult;
}

bool Downloader::GetLevel(const QString &strTargetFolder, const QUrl &urlLevel)
{
    // Set type of download
    bModUnpack = false;
    bGameUnpack = false;
    iPassCount = 0;
    // Download
    bool bResult = GetMain( strTargetFolder, urlLevel, false);
    return bResult;
}

bool Downloader::GetMod(const QString &strTargetFolder, const QString &strTargetGame, const QUrl &urlModBin, const QUrl &urlModData)
{
    bool bResult;
    // Save Taeget folders and urls
    strFirstTargetFolder = strTargetFolder;
    strFirstTargetUrl = urlModBin;
    strSecondTargetFolder = strTargetFolder + strTargetGame;
    strSecondTargetUrl = urlModData;
    // Choose type of download (When mod no need libraries)
    if(urlModBin.isEmpty()){
        // Set type of download
        bModUnpack = true;
        bGameUnpack = false;
        iPassCount = 0;
        // Download
        bResult = GetMain( "/tmp" , urlModData, true);
    } else {
        // Set type of download
        bModUnpack = true;
        bGameUnpack = false;
        iPassCount = 1;
        // Download
        bResult = GetMain( "/tmp" , urlModBin, true);
    }
    return bResult;
}

bool Downloader::GetGameSettings(const QString &strTargetFolder, const QUrl &urlGameBin, const QUrl &urlGameSettings)
{
    // Save Taeget folders and urls
    strFirstTargetFolder = strTargetFolder;
    strFirstTargetUrl = urlGameBin;
    strSecondTargetFolder = strTargetFolder;
    strSecondTargetUrl = urlGameSettings;
    // Set type of download
    bModUnpack = false;
    bGameUnpack = true;
    iPassCount = 1;
    // Download
    bool bResult = GetMain( "/tmp", urlGameBin, true);
    return bResult;
}

// Main function
bool Downloader::GetMain(const QString &strTargetFolder, const QUrl &url, bool unpack)
{
    if (strTargetFolder.isEmpty() || url.isEmpty())
    {
        return false;
    }

    // Create an object of the file class for download
    // here is the target directory and filename which is extracted from the url
    m_file = new QFile(strTargetFolder + QDir::separator() + url.fileName());
    // Trying to open a file
    if (!m_file->open(QIODevice::WriteOnly))
    {
        delete m_file;
        m_file = nullptr;
        return false;
    }

    // lock some actions
    locked_action = true;
    // Set label
    QString StrLabel = "Download: " + url.fileName();
    emit signalSetDownloadLabel(StrLabel);
    // Create a request
    QNetworkRequest request(url);
    // Be sure to allow redirects
    // deprecated:
    //request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    // Starting the download
    m_currentReply = m_manager.get(request);

    // After that, we immediately connect to the signals about the readiness of the data for reading and updating the download progress
    connect(m_currentReply, &QNetworkReply::readyRead, this, &Downloader::slotOnReadyRead);
    connect(m_currentReply, &QNetworkReply::downloadProgress, this, &Downloader::signalUpdateDownloadProgress);
    return true;
}

void Downloader::slotOnReadyRead()
{
    // If there is data and the file is open
    if (m_file)
    {
        // write them to a file
        m_file->write(m_currentReply->readAll());
    }
}

void Downloader::slotCancelDownload()
{
    locked_cancel = true;
    // Request cancellation
    if (m_currentReply)
    {
        m_currentReply->abort();
    }
}

void Downloader::slotOnReply(QNetworkReply* netReply)
{
    // Upon request completion
    if (netReply->error() == QNetworkReply::NoError)
    {
        // save the file
        m_file->flush();
        m_file->close();
    }
    else
    {
        // Or delete it in case of an error
        m_file->remove();
    }

    delete m_file;
    m_file = nullptr;
    netReply->deleteLater();

    // Download Progres End
    if(bGameUnpack || bModUnpack){
        if (bGameUnpack && iPassCount == 1){           
            m_unpack.Extract("/tmp/" + strFirstTargetUrl.fileName(), "/tmp/", 1, 0);
            m_copydir.CopyAndReplaceFolderContents("/tmp/x64", strFirstTargetFolder, false);
            iPassCount = 0;
            GetMain("/tmp/", strSecondTargetUrl, false);

        } else if (bGameUnpack && iPassCount == 0){
            m_unpack.Extract("/tmp/" + strSecondTargetUrl.fileName(), strSecondTargetFolder + "/", 1, 0);
            bGameUnpack = bModUnpack = false;
            emit signalDownloadProgressEnd();
            emit signalTestGameInstall();
            locked_action = false;
        }
        if (bModUnpack && iPassCount == 1){
            m_unpack.Extract("/tmp/" + strFirstTargetUrl.fileName(), "/tmp/", 1, 0);
            m_copydir.CopyAndReplaceFolderContents("/tmp/x64",strFirstTargetFolder, false);
            iPassCount = 0;
            GetMain("/tmp/", strSecondTargetUrl, false);

        } else if (bModUnpack && iPassCount == 0){
            m_unpack.Extract("/tmp/" + strSecondTargetUrl.fileName(), strSecondTargetFolder + "/", 1, 0);
            bGameUnpack = bModUnpack = false;
            emit signalDownloadProgressEnd();
            locked_action = false;
        }
    } else {
        emit signalDownloadProgressEnd();
        locked_action = false;
    }
}
