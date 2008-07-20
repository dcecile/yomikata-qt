#ifndef FILEINFO_H
#define FILEINFO_H

#include <QString>

class FileInfo
{
public:
    enum ArchiveType {
            Tar,
            TarGz,
            TarBz,
            Zip,
            Rar,
            InvalidArchiveType
    };
    static bool isImageFile(const QString& filename);
    static bool isArchiveFile(const QString& filename);
    static ArchiveType getArchiveType(const QString& filename);
    static QString getFileDialogWildcardString();

private:
    static const char *IMAGE_TYPES[];
    struct ArchiveTypeInfo {
        char *ext;
        ArchiveType type;
    };
    static const ArchiveTypeInfo ARCHIVE_TYPES[];
};

#endif
