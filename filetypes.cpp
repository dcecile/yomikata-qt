#include "filetypes.h"

FileType FileTypes::determineType(const KUrl& url)
{
    QString string = url.url();

    if (string.endsWith(".jpg")) {
        return Image;
    } else {
        return Invalid;
    }
}
