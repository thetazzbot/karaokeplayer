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

#include "util.h"
#include "logger.h"

#include <QByteArray>

#include "libuchardet/uchardet.h"


QTextCodec * Util::detectEncoding( const QByteArray &data )
{
    QTextCodec * codec = 0;

    uchardet_t ucd = uchardet_new();

    if ( uchardet_handle_data( ucd, data.constData(), data.size() ) == 0 )
    {
        // Notify an end of data to an encoding detctor.
        uchardet_data_end( ucd );

        const char * encoding = uchardet_get_charset( ucd );

        if ( encoding[0] != '\0' )
        {
            codec = QTextCodec::codecForName( encoding );
        }
    }

    uchardet_delete( ucd );
    return codec;
}

QString Util::convertEncoding(const QByteArray &data)
{
    QTextCodec * codec = detectEncoding( data );

    if ( !codec )
    {
        qDebug("Util: failed to detect the data encoding");
        return "";
    }

    qDebug("Util: automatically detected encoding %s", codec->name().constData() );
    return codec->toUnicode( data );
}
