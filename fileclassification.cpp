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

    /*for (i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++)
    {
        str.append('*');
        str.append(ARCHIVE_TYPES[i].ext);
        str.append(' ');
    }*/

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
    /*for (i = 0; *ARCHIVE_TYPES[i].ext != '\0'; i++)
    {
        str.append('*');
        str.append(ARCHIVE_TYPES[i].ext);
        str.append(' ');
    }*/

    str.append('|');
    str.append("Archive Files");

    return str;
}
