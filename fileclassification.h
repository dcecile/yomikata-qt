#ifndef FILECLASSICATION_H
#define FILECLASSICATION_H

#include <QString>

class FileClassification
{
public:
    enum ArchiveType {
        Tar,
        TarGz,
        TarBz,
        TarZ,
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
        const char *ext;
        ArchiveType type;
    };
    static const ArchiveTypeInfo ARCHIVE_TYPES[];

private:
    FileClassification();
};

#endif
