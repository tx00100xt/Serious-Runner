#include <limits.h>
#include <sys/types.h>
#include <dirent.h>
#include "Classes/FindInHomeDir.h"

FindInHomeDir::FindInHomeDir(QObject* parent) :
    BaseClass(parent)
{
    // TODO
}

FindInHomeDir::~FindInHomeDir()
{

}

QString FindInHomeDir::FindFileInHomeDir(QString strFileName)
{
    QByteArray ba1 = strFileName.toLocal8Bit();
    const char *chrFile = ba1.data();
    QString strHomeDir = QDir::homePath();
    QByteArray ba2 = strHomeDir.toLocal8Bit();
    const char *chrSearchPath = ba2.data();

    iTestFileDone = 0;
    SearchInDir(chrSearchPath, (const char*)chrFile, 0);
    if ( iTestFileDone == 0 ){
        return "";
    }
    chrResultPath[strlen(chrResultPath)] = 0x00;
    QString strPath = QString::fromLocal8Bit(chrResultPath);
    return strPath;
}

void FindInHomeDir::SearchInDir(const char *chrName, const char *chrFile,int iIndent)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(chrName)))
        return;

    if ( iTestFileDone == 1 )
        return;

    while ((entry = readdir(dir)) != NULL) {
        if ( iTestFileDone == 1 )
           break;
        if (entry->d_type == DT_DIR) {
            char chrPath[2048];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(chrPath, sizeof(chrPath), "%s/%s", chrName, entry->d_name);
            SearchInDir(chrPath, chrFile, iIndent + 2);
        } else {
            int iTestFile = strncmp((const char *)entry->d_name, (const char *)chrFile, (size_t) 16 );
            if( iTestFile == 0) {
                memcpy(chrResultPath, chrName, 2047);
                iTestFileDone = 1;
                break;
            }
        }
    }
    closedir(dir);
}
