#include "toolbarwidget.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QSlider>
#include <QStyle>

#include "debug.h"
#include "steward.h"

const int ToolbarWidget::FRAME_WIDTH = 0;
const int ToolbarWidget::SLIDE_DURATION = 150;
const float ToolbarWidget::SLIDE_FRAMES_PER_SECOND = 60.0;

ToolbarWidget::ToolbarWidget(Steward &steward, QWidget *parent)
    : QFrame(parent), _steward(steward)
{
    // Connect to steward
    connect(&_steward, SIGNAL(pageChanged(int, int)), SLOT(pageChanged(int, int)));

    // Raised panel
    setFrameStyle(StyledPanel | QFrame::Raised);
    setLineWidth(FRAME_WIDTH);

    // No vertical expanding
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Top buttons
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->setSpacing(10);
        mainLayout->addLayout(layout);
        
        layout->addStretch();

        // Page label
        _page = new QLabel("Page 0 / 0", this);
        _page->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        _page->setEnabled(false);
        layout->addWidget(_page);

        // Go to
        QPushButton *goTo = makeButton("Go to");
        layout->addWidget(goTo);

        /*

        // Page previous
        QPushButton *previousPage = makeButton("Previous Page");
        connect(previousPage, SIGNAL(clicked()), SIGNAL(previousPage()));
        layout->addWidget(previousPage);
        layout->addStretch();

        // Page next
        QPushButton *nextPage = makeButton("Next Page");
        connect(nextPage, SIGNAL(clicked()), SIGNAL(nextPage()));
        layout->addWidget(nextPage);
        layout->addStretch();

        */

        // Page shift next
        QPushButton *shiftNextPage = makeButton("Shift Pages");
        connect(shiftNextPage, SIGNAL(clicked()), &_steward, SLOT(shiftNext()));
        layout->addWidget(shiftNextPage);

        // Rule
        layout->addStretch();
        QFrame *rule1 = new QFrame(this);
        rule1->setFrameShape(QFrame::VLine);
        rule1->setFrameShadow(QFrame::Sunken);
        layout->addWidget(rule1);
        layout->addStretch();

        /*

        // Fullscreen
        QPushButton *fullscreen = makeButton("Fullscreen");
        fullscreen->setCheckable(true);
        connect(fullscreen, SIGNAL(toggled(bool)), SIGNAL(fullscreen(bool)));
        layout->addWidget(fullscreen);
        layout->addStretch();

        */

        // Zoom
        QPushButton *zoom = makeButton("Zoom");
        layout->addWidget(zoom);

        // Zoom label
        QLabel *zoomLabel = new QLabel("1x", this);
        zoomLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        layout->addWidget(zoomLabel);

        // Rule
        layout->addStretch();
        QFrame *rule2 = new QFrame(this);
        rule2->setFrameShape(QFrame::VLine);
        rule2->setFrameShadow(QFrame::Sunken);
        layout->addWidget(rule2);
        layout->addStretch();

        /*

        // Open
        QPushButton *open = makeButton("Open");
        connect(open, SIGNAL(clicked()), SIGNAL(open()));
        layout->addWidget(open);
        layout->addStretch();

        // Bookmarks
        QPushButton *bookmarks = makeButton("Bookmarks");
        layout->addWidget(bookmarks);
        layout->addStretch();

        */

        // Settings
        QPushButton *settings = makeButton("Menu");
        layout->addWidget(settings);

        /*
        // Quit
        QPushButton *quit = makeButton("Quit");
        connect(quit, SIGNAL(clicked()), SIGNAL(quit()));
        layout->addWidget(quit);

        */
        layout->addStretch();
    }

    // Rule
    QFrame *rule = new QFrame(this);
    rule->setFrameShape(QFrame::HLine);
    rule->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(rule);

    // Go to controls
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->setSpacing(10);
        mainLayout->addLayout(layout);

        // Done
        QPushButton *done = makeButton("Done");
        layout->addWidget(done, 0);

        // Space
        layout->addStretch(1);

        // Slider
        _seek = new QSlider(Qt::Horizontal, this);
        _seek->setInvertedAppearance(true);
        _seek->setEnabled(false);
        layout->addWidget(_seek, 4);
        connect(_seek, SIGNAL(valueChanged(int)), &_steward, SLOT(setPage(int)));

        // Space
        layout->addStretch(1);
    }

    // Zoom controls

    // Get the target height
    _fullHeight = mainLayout->minimumSize().height();

    // Set up the animations
    _animation.setFrameRange(0, _fullHeight);
    _animation.setDuration(SLIDE_DURATION);
    _animation.setUpdateInterval(int((100.0f / SLIDE_FRAMES_PER_SECOND) + 0.5f));
    _isShowing = true;
    connect(&_animation, SIGNAL(frameChanged(int)), SLOT(setShownHeight(int)));
}

ToolbarWidget::~ToolbarWidget()
{
}

QPushButton *ToolbarWidget::makeButton(const QString &text)
{
    QPushButton *button = new QPushButton(text, this);
    QFontMetrics metrics = button->fontMetrics();
    int width = metrics.width(text);
    QStyle *style = button->style();
    width += 2 * (style->pixelMetric(QStyle::PM_ButtonMargin) + style->pixelMetric(QStyle::PM_DefaultFrameWidth));
    button->setMinimumWidth(width);
    button->setMaximumWidth(width);
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    return button;
}

void ToolbarWidget::pageChanged(int page, int total)
{
    //qDebug()<<"Page changed"<<page<<total;

    // Update the label
    _page->setEnabled(true);
    _page->setText(QString("Page %0 / %1").arg(page + 1).arg(total));

    // Update the slider
    _seek->setEnabled(true);
    _seek->setRange(0, total - 1);
    _seek->setValue(page);
}

void ToolbarWidget::startShow()
{
    if (!_isShowing)
    {
        _animation.toggleDirection();
        _isShowing = true;
    }

    if (_animation.state() == QTimeLine::NotRunning)
    {
        _animation.setCurrentTime(0);
        _animation.start();
    }
}

void ToolbarWidget::startHide()
{
    if (_isShowing)
    {
        _animation.toggleDirection();
        _isShowing = false;
    }

    if (_animation.state() == QTimeLine::NotRunning)
    {
        _animation.setCurrentTime(_animation.duration());
        _animation.start();
    }
}

void ToolbarWidget::setShownHeight(int height)
{
    if (height == 0)
    {
        hide();
    }
    else
    {
        show();
        setContentsMargins(FRAME_WIDTH, FRAME_WIDTH + height - _fullHeight, FRAME_WIDTH, FRAME_WIDTH);
    }
}
