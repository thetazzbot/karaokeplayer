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
#include <QTextCodec>

#include "lyricsparser.h"


LyricsParser::LyricsParser(  ConvertEncoding * converter )
{
    m_converter = converter;
}

LyricsParser::~LyricsParser()
{
}

QByteArray LyricsParser::load(QIODevice *file)
{
    QByteArray datalyrics = file->readAll();

    if ( datalyrics.indexOf( '\r') != -1 )
        datalyrics.replace( "\r\n", "\n" );

    if ( datalyrics.isEmpty() )
        throw("Lyrics file is empty");

    return datalyrics;
}

QString LyricsParser::timeAsText(quint64 timing)
{
    int min = timing / 60000;
    int sec = (timing - min * 60000) / 1000;
    int msec = timing - (min * 60000 + sec * 1000 );

    return QString().sprintf( "%02d:%02d.%04d", min, sec, msec );
}

QTextCodec *LyricsParser::detectEncoding(const QByteArray &data, LyricsLoader::Properties &properties )
{
    // Now detect encoding
    QTextCodec * enc = 0;

    //qDebug("Supplied text: %s", data.constData() );

    if ( m_converter )
        enc = (*m_converter)( data);

    if ( !enc )
        enc = QTextCodec::codecForName( "utf-8" );
    else
        properties[ LyricsLoader::PROP_DETECTED_ENCODING ] = enc->name();

    return enc;
}
