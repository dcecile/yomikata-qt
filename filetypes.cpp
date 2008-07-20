#include "filetypes.h"

FileType FileTypes::determineType(const QString& filename)
{
    if (filename.endsWith(".jpg")) {
        return Image;
    } else if (filename.endsWith(".rar")) {
        return Archive;
    } else {
        return Invalid;
    }
}
