#ifndef DEBUG_H
#define DEBUG_H

/**
 * @todo Raise an error if kDebug or qDebug is used.
 * @todo Resolve compatibility issues.
 */

#include <QDebug>
#define debug() qDebug()
#undef qDebug

#endif
