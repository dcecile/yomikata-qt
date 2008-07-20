#include <KDebug>
#include <KLocale>
#include <QImageReader>

#include "fileinfo.h"

const char *FileInfo::IMAGE_TYPES[] = {
    ".jpg",
    ".jpeg",
    ".png",
    ""
};
// Note: maybe QImageReader::supportedImageFormats() should be used for image formats
//  (has 20+ entries)

const FileInfo::ArchiveTypeInfo FileInfo::ARCHIVE_TYPES[] = {
    {".tar", Tar},
    {".tar.gz", TarGz},
    {".tgz", TarGz},
    {".tar.bz", TarBz},
    {".tar.bz2", TarBz},
    {".tbz2", TarBz},
    {".zip", Zip},
    {".cbz", Zip},
    {".rar", Rar},
    {".cbr", Rar},
    {"", InvalidArchiveType},
};

bool FileInfo::isImageFile(const QString& filename)
{
    for (int i = 0; *IMAGE_TYPES[i] != '\0'; i++) {
        if (filename.endsWith(IMAGE_TYPES[i])) {
            return true;
        }
    }
    return false;
}
bool FileInfo::isArchiveFile(const QString& filename)
{
    for (int i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++) {
        if (filename.endsWith(ARCHIVE_TYPES[i].ext)) {
            return true;
        }
    }
    return false;
}
FileInfo::ArchiveType FileInfo::getArchiveType(const QString& filename)
{
    for (int i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++) {
        if (filename.endsWith(ARCHIVE_TYPES[i].ext)) {
            return ARCHIVE_TYPES[i].type;
        }
    }
    return InvalidArchiveType;
}
QString FileInfo::getFileDialogWildcardString()
{
    int i;
    QString str;

    // All supported files
    for (i = 0; *IMAGE_TYPES[i] != '\0'; i++) {
        str.append('*');
        str.append(IMAGE_TYPES[i]);
        str.append(' ');
    }
    for (i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++) {
        str.append('*');
        str.append(ARCHIVE_TYPES[i].ext);
        str.append(' ');
    }
    str.append('|');
    str.append(i18n("All Supported Files"));
    str.append('\n');

    // Image files
    for (i = 0; *IMAGE_TYPES[i] != '\0'; i++) {
        str.append('*');
        str.append(IMAGE_TYPES[i]);
        str.append(' ');
    }
    str.append('|');
    str.append(i18n("Image Files"));
    str.append('\n');

    // Archive files
    for (i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++) {
        str.append('*');
        str.append(ARCHIVE_TYPES[i].ext);
        str.append(' ');
    }
    str.append('|');
    str.append(i18n("Archive Files"));

    return str;
}
