#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <QObject>

class Archive : public QObject
{
    Q_OBJECT

public:
    enum Type
    {
        Tar,
        TarGz,
        TarBz,
        TarZ,
        Zip,
        Rar,
        SevenZip,
        InvalidArchiveType
    };

public:
    Archive(QObject *parent = NULL);
    ~Archive();

    void reset(const QString &_fileName);

    const QString &filename() const;
    Type type() const;
    const QString &programPath() const;

private:
    QString _filename;
    Type _type;
    QString _programPath;
};

#endif
