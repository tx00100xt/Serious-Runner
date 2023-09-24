#include <QtCore/qiodevice.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <archive.h>
#include <archive_entry.h>
#include "Classes/Unpacker.h"
#include "Classes/Types.h"

Unpacker::Unpacker(QObject* parent) :
    BaseClass(parent)
{
    // TODO
}

Unpacker::~Unpacker()
{

}

// Function interacting with the libarchive and liblzma (xz-utils) libraries
void Unpacker::Extract(QString strFileName, QString strPath, int iDoExtract, int iFlags)
{
    int r;
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    char chrPathWithFile[4097];
    char chrPathForEmit[4097];

    iFlags = ARCHIVE_EXTRACT_TIME;
    iFlags |= ARCHIVE_EXTRACT_PERM;
    iFlags |= ARCHIVE_EXTRACT_ACL;
    iFlags |= ARCHIVE_EXTRACT_FFLAGS;

    a = archive_read_new();
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, iFlags);
    archive_read_support_filter_xz(a);
    archive_read_support_filter_bzip2(a);
    archive_read_support_filter_gzip(a);
    archive_read_support_format_zip(a);
    archive_read_support_filter_compress(a);
    archive_read_support_format_tar(a);
    archive_read_support_format_cpio(a);   
    archive_read_support_format_7zip(a);
    archive_write_disk_set_standard_lookup(ext);

    QByteArray ba1 = strFileName.toLocal8Bit();
    QByteArray ba2 = strPath.toLocal8Bit();
    const char *chrFilename = ba1.data();
    const char *chrPath = ba2.data();

    if (chrFilename != NULL && strcmp(chrFilename, "-") == 0)
        chrFilename = NULL;
    if ((r = archive_read_open_filename(a, chrFilename, 10240))) {
        MsgBox(ERROR, QString::fromLocal8Bit(archive_error_string(a)));
        return;
    }
    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r != ARCHIVE_OK) {
            MsgBox(ERROR, QString::fromLocal8Bit(archive_error_string(a)));
            return;
        }

        if (iDoExtract) { // msg(archive_entry_pathname(entry)); --> pathname
            const char* chrCurrentFile = archive_entry_pathname(entry);
            memset(chrPathWithFile,0,4096);
            strcat(chrPathWithFile, chrPath);
            strcat(chrPathWithFile, chrCurrentFile);

            strFile = QString::fromLocal8Bit(chrCurrentFile);
            emit signalCurrentUnpackFile("Unpack: " + strFile);
            archive_entry_set_pathname(entry, (const char*)chrPathWithFile);
            r = archive_write_header(ext, entry);
            if (r != ARCHIVE_OK) {
                MsgBox(ERROR, QString::fromLocal8Bit(archive_error_string(a)));
            }
            else {
                r = CopyData(a, ext);
                if (r != ARCHIVE_OK) {};
            }
        }
    }
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    emit signalUnpackEnd();
    return;
}

// Internal Unpacker function
int Unpacker::CopyData(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
    int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r != ARCHIVE_OK) {
            MsgBox(ERROR, QString::fromLocal8Bit(archive_error_string(ar)));
            return (r);
        }
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK) {
            MsgBox(ERROR, QString::fromLocal8Bit(archive_error_string(ar)));
            return (r);
        }
    }
}

// MsgBox
void Unpacker::MsgBox(int type, QString strMessage)
{
    switch(type)
    {
        case INFO:
             msgBox.setIconPixmap(QPixmap(":/Images/ok.png"));
            break;
        case WARN:
             msgBox.setIconPixmap(QPixmap(":/Images/warning.png"));
            break;
        case 2:
             msgBox.setIconPixmap(QPixmap(":/Images/error.png"));
            break;
        default:
            break;
    }
    msgBox.setText(strMessage);
    msgBox.setWindowTitle("Unpacker");
    msgBox.exec();
}
