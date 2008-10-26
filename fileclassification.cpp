#include "fileclassification.h"

/**
 * @todo Use QImageReader::supportedImageFormats()
 */
const char *FileClassification::IMAGE_TYPES[] = {
    ".jpg",
    ".jpeg",
    ".png",
    ".gif",
    ".bmp",
    ".tiff",
    ""
};

const FileClassification::ArchiveTypeInfo FileClassification::ARCHIVE_TYPES[] = {
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

bool FileClassification::isImageFile(const QString& filename)
{
    for (int i = 0; *IMAGE_TYPES[i] != '\0'; i++)
    {
        if (filename.endsWith(IMAGE_TYPES[i], Qt::CaseInsensitive))
        {
            return true;
        }
    }
    return false;
}

bool FileClassification::isArchiveFile(const QString& filename)
{
    for (int i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++)
    {
        if (filename.endsWith(ARCHIVE_TYPES[i].ext, Qt::CaseInsensitive))
        {
            return true;
        }
    }
    return false;
}

FileClassification::ArchiveType FileClassification::getArchiveType(const QString& filename)
{
    for (int i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++)
    {
        if (filename.endsWith(ARCHIVE_TYPES[i].ext, Qt::CaseInsensitive))
        {
            return ARCHIVE_TYPES[i].type;
        }
    }
    return InvalidArchiveType;
}

QString FileClassification::getFileDialogWildcardString()
{
    int i;
    QString str;

    // All supported files
    for (i = 0; *IMAGE_TYPES[i] != '\0'; i++)
    {
        str.append('*');
        str.append(IMAGE_TYPES[i]);
        str.append(' ');
    }

    for (i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++)
    {
        str.append('*');
        str.append(ARCHIVE_TYPES[i].ext);
        str.append(' ');
    }

    str.append('|');
    str.append("All Supported Files");
    str.append('\n');

    // Image files
    for (i = 0; *IMAGE_TYPES[i] != '\0'; i++)
    {
        str.append('*');
        str.append(IMAGE_TYPES[i]);
        str.append(' ');
    }

    str.append('|');
    str.append("Image Files");
    str.append('\n');

    // Archive files
    for (i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++)
    {
        str.append('*');
        str.append(ARCHIVE_TYPES[i].ext);
        str.append(' ');
    }

    str.append('|');
    str.append("Archive Files");

    return str;
}
