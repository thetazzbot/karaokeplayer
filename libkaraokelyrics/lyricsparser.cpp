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

#include "lyricsparser.h"


LyricsParser::LyricsParser()
{
}

LyricsParser::~LyricsParser()
{
}

QStringList LyricsParser::loadText(QIODevice * file)
{
    // Other formats are text files, so we need to find out the encoding
    // FIXME: assuming UTF-8
    QByteArray datalyrics = file->readAll();

    if ( datalyrics.indexOf( '\r') != -1 )
        datalyrics.replace( "\r\n", "\n" );

    return QString::fromUtf8( datalyrics ).split( '\n' );
}

QString LyricsParser::timeAsText(quint64 timing)
{
    int min = timing / 60000;
    int sec = (timing - min * 60000) / 1000;
    int msec = timing - (min * 60000 + sec * 1000 );

    return QString().sprintf( "%02d:%02d.%04d", min, sec, msec );
}
