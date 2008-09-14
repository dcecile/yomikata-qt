#ifndef DEBUG_H
#define DEBUG_H

/**
 * @todo Raise an error if kDebug or qDebug is used.
 * @todo Resolve compatibility issues.
 */

#ifdef QT_ONLY
    #include <QDebug>
    #define debug() qDebug()
#else
    #include <KDebug>
    #define debug() kDebug()
#endif


#endif
