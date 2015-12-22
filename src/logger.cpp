#include <stdarg.h>
#include <stdio.h>

#include "logger.h"

Logger::Logger( QObject *parent )
    : QObject(parent)
{
}

void Logger::debug(const QString &str)
{
    qDebug("%s", qPrintable(str) );
}

void Logger::debug(const char *fmt, ... )
{
    va_list vl;
    char buf[1024];

    va_start( vl, fmt );
    vsnprintf( buf, sizeof(buf-1), fmt, vl );
    va_end( vl );

    qDebug( "%s", buf );
}
