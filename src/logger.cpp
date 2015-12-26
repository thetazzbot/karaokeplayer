/**************************************************************************
 *  Ulduzsoft Karaoke PLayer - cross-platform desktop karaoke player      *
 *  Copyright (C) 2015-2016 George Yunaev, support@ulduzsoft.com          *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *																	      *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

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
    vsnprintf( buf, sizeof(buf) - 1, fmt, vl );
    va_end( vl );

    qDebug( "%s", buf );
}
