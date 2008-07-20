#include "fileclassifier.h"

#include <KDebug>
#include <KLocale>
#include <QImageReader>



// Note: http://en.wikipedia.org/wiki/CDisplay_RAR_Archived_Comic_Book_File
//  talks about the "comic book archive" format

const char *FileClassifier::IMAGE_TYPES[] = {
    ".jpg",
    ".jpeg",
    ".png",
    ".gif",
    ".bmp",
    ".tiff",
    ""
};
// Note: maybe QImageReader::supportedImageFormats() should be used for image formats
//  (has 20+ entries)

const FileClassifier::ArchiveTypeInfo FileClassifier::ARCHIVE_TYPES[] = {
    {".tar", Tar},
    {".tar.gz", TarGz},
    {".tgz", TarGz},
    {".tar.bz", TarBz},
    {".tar.bz2", TarBz},
    {".tbz2", TarBz},
    {".tar.Z", TarZ}, // Using "compress"
    {".zip", Zip},
    {".cbz", Zip},
    {".rar", Rar},
    {".cbr", Rar},
    // Ace
    // 7zip
    {"", InvalidArchiveType},
};

bool FileClassifier::isImageFile(const QString& filename)
{
    for (int i = 0; *IMAGE_TYPES[i] != '\0'; i++) {
        if (filename.endsWith(IMAGE_TYPES[i], Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}
bool FileClassifier::isArchiveFile(const QString& filename)
{
    for (int i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++) {
        if (filename.endsWith(ARCHIVE_TYPES[i].ext, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}
FileClassifier::ArchiveType FileClassifier::getArchiveType(const QString& filename)
{
    for (int i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++) {
        if (filename.endsWith(ARCHIVE_TYPES[i].ext, Qt::CaseInsensitive)) {
            return ARCHIVE_TYPES[i].type;
        }
    }
    return InvalidArchiveType;
}
QString FileClassifier::getFileDialogWildcardString()
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

#include "fileclassifier.moc"
