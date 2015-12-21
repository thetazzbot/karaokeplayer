#include "logger.h"

Logger::Logger( QObject *parent )
    : QObject(parent)
{
}

void Logger::debug(const QString &str)
{
    qDebug("%s", qPrintable(str) );
}
