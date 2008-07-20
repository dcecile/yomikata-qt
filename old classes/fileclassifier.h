#ifndef FILECLASSIFIER_H
#define FILECLASSIFIER_H

#include <QObject>

/**
 * @ingroup mod_source
 * @brief Classifies files as images or archives.
*/
class FileClassifier : public QObject
{
    Q_OBJECT

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
        char *ext;
        ArchiveType type;
    };
    static const ArchiveTypeInfo ARCHIVE_TYPES[];
};

#endif
