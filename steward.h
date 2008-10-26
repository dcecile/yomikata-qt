#ifndef STEWARD_H
#define STEWARD_H

#include <QObject>

#include <QPixmap>

class QWidget;

class Book;
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

    QWidget *projector();
    QWidget *debugWidget();

    void reset(const QString &filename);

public slots:
    void next();
    void previous();
    void shiftNext();

private slots:
    void indexerBuilt();
    void pageChanged();
    void decodeDone(int index, QPixmap page);
    void viewportResized(const QSize &size);

private:
    Book &_book;
    Indexer &_indexer;
    Strategist &_strategist;
    Artificer &_artificer;
    Projector &_projector;

    QWidget *_debugWidget;

    bool _buildingIndexer;
};

#endif
