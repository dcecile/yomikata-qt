#ifndef EXTRACTDECODETHREAD_H
#define EXTRACTDECODETHREAD_H

#include "decodethread.h"

#include <QStringList>

#include "planner.h"
#include "fileclassifier.h"

/**
 * @ingroup mod_decoder
 */
class ExtractDecodeThread : public DecodeThread
{
    Q_OBJECT

public:
    ExtractDecodeThread(FileClassifier::ArchiveType archiveType, const QString &archive, QObject *parent);

    ~ExtractDecodeThread();

private:
    void decode();

private:
    QString _command;
    QStringList _args;
};

#endif
