#ifndef YOMIKATA_H
#define YOMIKATA_H

#include <KMainWindow>
#include <KXmlGuiWindow>
#include <KTextEdit>
#include <QLabel>
#include <QBoxLayout>
#include <KAction>
#include <KSelectAction>
#include <QBasicTimer>

#include "pagecache.h"
#include "filelister.h"
#include "extractlister.h"
#include "filedecodethread.h"
#include "extractdecodethread.h"
#include "pagedisplay.h"

class Yomikata : public KXmlGuiWindow
{
    Q_OBJECT

public:
    Yomikata(const QString &initialArg, QWidget *parent=0);
    ~Yomikata();

private slots:
    void open();
    void toggleFullScreen(bool checked);
    void toggleMenubar(bool checked);

    void pageReadFailed(int pageNum);

    void enableForward(bool enabled);
    void enableBackward(bool enabled);

    void enableZoomToggle(bool enabled);
    void enableZoomIn(bool enabled);
    void enableZoomOut(bool enabled);

    void setSinglePageView();
    void setComicsView();
    void setMangaView();

private:
    void startListing(const QString &initialFile);

    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);

    void createActions();

    void setAppDefaults();

private:
    // Current state
    enum PageMode {SingleMode=0, ComicsMode, MangaMode};

    // Loaded modules
    PageCache _pageCache;
    FileLister _fileLister;
    ExtractLister _extractLister;
    FileDecodeThread _fileDecodeThread;
    ExtractDecodeThread _extractDecodeThread;
    PageDisplay _pageDisplay;

    // Actions
    QAction *_pageForwardAction;
    QAction *_pageBackwardAction;
    KAction *_pageLeftAction;
    KAction *_pageRightAction;
    KAction *_pageToStartAction;
    KAction *_pageToEndAction;

    KAction *_zoomToggleAction;
    KAction *_zoomInAction;
    KAction *_zoomOutAction;

    KSelectAction *_viewSelectAction;
    KAction *_singlePageAction;
    KAction *_comicsViewAction;
    KAction *_mangaViewAction;

    // Loading stuff
    bool _archiveMode;
    QString _archivePath;
    FileInfo::ArchiveType _archiveType;
};

#endif
