#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <QObject>
#include <QSettings>

class Archive : public QObject
{
    Q_OBJECT

public:
    enum Type
    {
        SevenZip = 0,
        Tar,
        Zip,
        Rar,
        InvalidArchiveType
    };

public:
    Archive(QObject *parent = NULL);
    ~Archive();

    void testPrograms();

    void reset(const QString &_fileName);

    const QString &filename() const;
    Type type() const;
    const QString &programPath() const;

private:
    QSettings _settings;
    bool _programExists[InvalidArchiveType];
    QString _programPaths[InvalidArchiveType];
    bool _sevenZipRarExists;
    QString _filename;
    Type _type;
};

#endif
