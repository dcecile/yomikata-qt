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

#include <vector>

#include "source.h"
#include "book.h"
#include "steward.h"
#include "artificer.h"

using std::vector;

/**
 * @defgroup mod_userinterface User Interface module
 * This module handles input from the user.
 */

/**
 * @ingroup mod_userinterface
 * @brief The program's main window
 *
 * Yomikata manages all QAction objects and passes input to Oyabun and Presenter.
 */

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

    void enableForward(bool enabled);
    void enableBackward(bool enabled);

    void enableZoomToggle(bool enabled);
    void enableZoomIn(bool enabled);
    void enableZoomOut(bool enabled);

    void setSinglePageView();
    void setComicsView();
    void setMangaView();

private:
    void setSource(const QString &initialFile);

    void startListing(const QString &initialFile);

    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);

    void createActions();

    void setAppDefaults();

private:
    // Current state
    enum PageMode {SingleMode=0, ComicsMode, MangaMode};

    // Object set
    Source _source;
    Book _book;
    Projector _projector;
    Steward _steward;
    vector<Artificer> _artifcers;

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
};

#endif
