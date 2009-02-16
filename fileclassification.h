#ifndef FILECLASSICATION_H
#define FILECLASSICATION_H

#include <QString>

class FileClassification
{
public:
    static bool isImageFile(const QByteArray& filename);
    static QString getFileDialogWildcardString();

private:
    static const char *IMAGE_TYPES[];

private:
    FileClassification();
};

#endif
