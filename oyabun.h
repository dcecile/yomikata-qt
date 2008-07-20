#ifndef OYABUN_H
#define OYABUN_H

#include <QObject>
#include <QTime>

#include "source.h"
#include "filelist.h"
#include "book.h"
#include "decoder.h"
#include "depicter.h"

/**
 * @defgroup mod_oyabun Oyabun module
 * This is the oyabun group.
 */

/**
 * @ingroup mod_oyabun
 * @short The lazy oyabun; he will interact as little as possible with his buka and delegate as much as he can to them.
 *
 * Oyabun coordinates the display of new pages when the user changes
 * pages. It also makes sure that the cache gets filled during spare
 * time.
 *
 * It keeps Yomikata notified about whether the page can be turned
 * forward or backward.
 *
 * It runs in the same thread as the main window because it will delegate
 * expensive tasks to other threads.
 *
 */
class Oyabun : public QObject
{
    Q_OBJECT

public:
    Oyabun(QWidget *parent);
    QWidget *getDepicter() { return &_depicter; }

    void start(const QString &initialFile);

public slots:
    void turnPageForward();
    void turnPageBackward();
    void turnPageForwardOnePage();
    void turnPageToStart();
    void turnPageToEnd();

signals:
    void forwardEnabled(bool enabled);
    void backwardEnabled(bool enabled);

private slots:
    void donePopulatingFileList(const vector<FileInfo> &files);
    void decodeDone();
    void currentPagesReset();

private:
    void enableForward(bool enabled);
    void enableBackward(bool enabled);

    void changePages();

    void precache();

private:
    Source _source;
    FileList _fileList;
    Book _book;
    Decoder _decoder;
    Depicter _depicter;

    int _numPages;
    QTime _requestTime[2];
    int _bufferStart;
    int _bufferEnd;
    bool _forwardEnabled;
    bool _backwardEnabled;
};

#endif
