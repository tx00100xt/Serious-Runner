#include <QDir>
#include <QFile>
#include <QDirIterator>
#include "Classes/CopyDir.h"

CopyDir::CopyDir(QObject* parent) :
    BaseClass(parent)
{
    // Connecting to the signal
}

CopyDir::~CopyDir()
{

}

void CopyDir::CopyAndReplaceFolderContents(const QString &strFromDir, const QString &strToDir, bool bCopyAndRemove)
{   

    QDirIterator it(strFromDir, QDirIterator::Subdirectories);
    QDir dir(strFromDir);
    const int absSourcePathLength = dir.absoluteFilePath(strFromDir).length();
    //m_dialog.exec();

    while (it.hasNext()){
        it.next();
        const auto fileInfo = it.fileInfo();
        if(!fileInfo.isHidden()) { //filters dot and dotdot
            const QString subPathStructure = fileInfo.absoluteFilePath().mid(absSourcePathLength);
            const QString constructedAbsolutePath = strToDir + subPathStructure;

            if(fileInfo.isDir()){
                //Create directory in target folder
                dir.mkpath(constructedAbsolutePath);
            } else if(fileInfo.isFile()) {
                //Copy File to target directory

                //Remove file at target location, if it exists, or QFile::copy will fail
                emit signalCopyDir("Copy: " + constructedAbsolutePath);
                QFile::remove(constructedAbsolutePath);
                QFile::copy(fileInfo.absoluteFilePath(), constructedAbsolutePath);
            }
        }
    }

    if(bCopyAndRemove){
        dir.removeRecursively();
    }
    emit signalCopyDirEnd();
}

bool CopyDir::CopyPath(QString strSourceDir, QString strDestinationDir, bool bOverWriteDirectory)
{
    QDir originDirectory(strSourceDir);

    if (! originDirectory.exists())
    {
        return false;
    }

    QDir destinationDirectory(strDestinationDir);

    if(destinationDirectory.exists() && !bOverWriteDirectory)
    {
        return false;
    }
    else if(destinationDirectory.exists() && bOverWriteDirectory)
    {
        destinationDirectory.removeRecursively();
    }

    originDirectory.mkpath(strDestinationDir);

    foreach (QString strDirectoryName, originDirectory.entryList(QDir::Dirs | \
                                                              QDir::NoDotAndDotDot))
    {
        QString strDestinationPath = strDestinationDir + "/" + strDirectoryName;
        originDirectory.mkpath(strDestinationPath);
        CopyPath(strSourceDir + "/" + strDirectoryName, strDestinationPath, bOverWriteDirectory);
    }

    foreach (QString strFileName, originDirectory.entryList(QDir::Files))
    {
        QFile::copy(strSourceDir + "/" + strFileName, strDestinationDir + "/" + strFileName);
    }

    /*! Possible race-condition mitigation? */
    QDir finalDestination(strDestinationDir);
    finalDestination.refresh();

    if(finalDestination.exists())
    {
        return true;
    }

    return false;
}
