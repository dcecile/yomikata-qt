#ifndef STEWARD_H
#define STEWARD_H

#include <QObject>

#include <QPixmap>
#include <QMutex>

class Book;
class Archive;
class Indexer;
class Strategist;
class Artificer;
class Projector;

/**
 * @todo Use QAction for actions
 */
class Steward : public QObject
{
    Q_OBJECT

public:
    Steward(QObject *parent = NULL);
    ~Steward();

    QWidget *debugWidget();

    void reset(const QString &filename);

    void setViewSize(const QSize &size);
    void paintView(QPainter *painter, const QRect &updateRect);

    void mouseMoved(const QPointF &pos);

public slots:
    void next();
    void previous();
    void shiftNext();
    void setPage(int page);

signals:
    void viewUpdate();
    void viewRepaint();
    void pageChanged(int page, int total);

private slots:
    void indexerBuilt();
    void decodeDone(int index, QPixmap page);
    void recievedFullPageSize(int index);
    void dualCausedPageChange();

private:
    void pageChanged();
    void loadPages();

private:
    QMutex _lock;
    Book &_book;
    Archive &_archive;
    Indexer &_indexer;
    Strategist &_strategist;
    Artificer &_artificer;
    Projector &_projector;

    bool _buildingIndexer;
};

#endif
