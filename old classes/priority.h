#ifndef PRIORITY_H
#define PRIORITY_H

#include <QObject>

/**
 * @defgroup mod_priority Priority module
 *
 */

/**
 * @ingroup mod_priority
 * @brief Compares the priority of pages
 */
class Priority : public QObject
{
    Q_OBJECT

public:
    Priority(QObject *parent);

    ~Priority();

};

#endif
