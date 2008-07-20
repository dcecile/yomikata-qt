#ifndef SOURCE_H
#define SOURCE_H

#include <QObject>

#include "fileclassifier.h"
#include "planner.h"

class Lister;
class DecodeThread;

/**
 * @defgroup mod_source Source module
 * Stores information related to the source.
 */

/**
 * @ingroup mod_source
 * @brief The source file
 *
 * Stores the name of the initial file and information about the file.
 *
 * Creates classes that depend on the file info.
 *
 */
class Source : public QObject
{
    Q_OBJECT

public:
    Source(QObject *parent);

    void set(const QString &initialFile);

    Lister *createLister();
    DecodeThread *createDecodeThread();

    ~Source();

private:
    QString _fileName;
    bool _isArchive;
    FileClassifier::ArchiveType _archiveType;
};

#endif
