#include "toolbarwidget.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QSlider>

ToolbarWidget::ToolbarWidget(QWidget *parent)
    : QWidget(parent)
{
    // No vertical expanding
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Top buttons
    {
        QHBoxLayout *layout = new QHBoxLayout();
        mainLayout->addLayout(layout);

        // Go to
        QPushButton *goTo = new QPushButton("Go to", this);
        goTo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->addWidget(goTo);

        // Page label
        QLabel *page = new QLabel("Page 3 / 50", this);
        page->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->addWidget(page);

        // Rule
        QFrame *rule1 = new QFrame(this);
        rule1->setFrameShape(QFrame::VLine);
        rule1->setFrameShadow(QFrame::Sunken);
        layout->addStretch();
        layout->addWidget(rule1);
        layout->addStretch();

        // Zoom
        QPushButton *zoom = new QPushButton("Zoom", this);
        zoom->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->addWidget(zoom);

        // Zoom label
        QLabel *zoomLabel = new QLabel("1x", this);
        zoomLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->addWidget(zoomLabel);

        // Rule
        QFrame *rule2 = new QFrame(this);
        rule2->setFrameShape(QFrame::VLine);
        rule2->setFrameShadow(QFrame::Sunken);
        layout->addStretch();
        layout->addWidget(rule2);
        layout->addStretch();

        // Open
        QPushButton *open = new QPushButton("Open", this);
        open->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(open, SIGNAL(clicked()), SIGNAL(open()));
        layout->addWidget(open);

        // Bookmarks
        QPushButton *bookmarks = new QPushButton("Bookmarks", this);
        bookmarks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->addWidget(bookmarks);

        // Settings
        QPushButton *settings = new QPushButton("Settings", this);
        settings->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->addWidget(settings);

        // Quit
        QPushButton *quit = new QPushButton("Quit", this);
        quit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(quit, SIGNAL(clicked()), SIGNAL(quit()));
        layout->addWidget(quit);
    }

    // Rule
    QFrame *rule = new QFrame(this);
    rule->setFrameShape(QFrame::HLine);
    rule->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(rule);

    // Go to controls
    {
        QHBoxLayout *layout = new QHBoxLayout();
        mainLayout->addLayout(layout);

        // Done
        QPushButton *done = new QPushButton("Done", this);
        done->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->addWidget(done, 0);

        // Space
        layout->addStretch(1);

        // Slider
        QSlider *slider = new QSlider(Qt::Horizontal, this);
        layout->addWidget(slider, 2);

        // Page label
        QLabel *page = new QLabel("Page 3 / 50", this);
        page->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        layout->addWidget(page, 0);

        // Space
        layout->addStretch(1);
    }

    // Zoom controls
}

ToolbarWidget::~ToolbarWidget()
{
}

#include "toolbarwidget.moc"
