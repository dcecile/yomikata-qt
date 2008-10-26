#include "projector.h"

#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QResizeEvent>
#include <QVBoxLayout>

#include "debug.h"

Projector::Projector(QWidget *parent)
    : QWidget(parent)
{
    // Initialize
    _loading0 = createLoadingWidget();
    _loading0->hide();
    _loading1 = createLoadingWidget();
    _loading1->hide();

    _page0 = new QLabel(this);
    _page0->hide();
    _page1 = new QLabel(this);
    _page1->hide();

    // Always expand
    QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);
}

Projector::~Projector()
{
}

QWidget *Projector::createLoadingWidget()
{
    QWidget *loading = new QWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(loading);
    layout->setAlignment(Qt::AlignHCenter);

    layout->addStretch(1);

    QLabel *label = new QLabel("Loading", this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    layout->addSpacing(20);

    QProgressBar *progress = new QProgressBar(this);
    progress->setRange(0, 0);
    progress->setMaximumWidth(100);
    layout->addWidget(progress);

    layout->addStretch(1);

    return loading;
}

int Projector::heightForWidth(int width)
{
    return width * 3 / 4;
}

QSize Projector::sizeHint() const
{
    return QSize(200, 100);
}

void Projector::showBlank()
{
    /*
    _loading0->hide();
    _loading1->hide();
    _page0->hide();
    _page1->hide();
    */
}

void Projector::showLoading0(const QRect &rect)
{
    _loading0->setGeometry(rect);

    _page0->hide();
    _loading0->show();
}

void Projector::showLoading1(const QRect &rect)
{
    _loading1->setGeometry(rect);

    _page1->hide();
    _loading1->show();
}

void Projector::showPage0(const QRect &rect, QPixmap image)
{
    _page0->setPixmap(image);
    _page0->setGeometry(rect);

    _loading0->hide();
    _page0->show();
}

void Projector::showPage1(const QRect &rect, QPixmap image)
{
    _page1->setPixmap(image);
    _page1->setGeometry(rect);

    _loading1->hide();
    _page1->show();
}

void Projector::resizeEvent(QResizeEvent *event)
{
    emit resized(event->size());
}

#include "projector.moc"
