#ifndef FILETYPES_H
#define FILETYPES_H

#include <KUrl>

enum FileType {
        Image,
        Archive,
        Invalid
};
class FileTypes
{
public:
    static FileType determineType(const KUrl& url);
};

#endif
