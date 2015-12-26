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

#include <QMap>
#include <QRegExp>
#include <QStringList>

#include "lyricsparser_lrc.h"


LyricsParser_LRC::LyricsParser_LRC()
{
    offset = 0;
}

void LyricsParser_LRC::parse( QIODevice * file, LyricsLoader::Container& output, LyricsLoader::Properties& properties )
{
    bool header = true;
    qint64 last_time = -1;

    QStringList lyrics = loadText( file );

    // We use a map to support LRC1 multitime lyrics
    QMap< qint64, QString > templyrics;

    Q_FOREACH ( QString line, lyrics )
    {
        // To simplify the matching
        line.replace( '<', '[' );
        line.replace( '>', ']' );

        if ( header )
        {
            QRegExp regex( "^\\[([a-zA-Z]+):\\s*(.*)\\s*\\]$" );
            regex.setMinimal( true );

            if ( regex.indexIn( line ) != -1 )
            {
                QString tag = regex.cap( 1 );
                QString value = regex.cap( 2 );

                if ( tag == "ti" )
                    properties[ LyricsLoader::PROP_TITLE ] = value;
                else if ( tag == "ar" )
                    properties[ LyricsLoader::PROP_ARTIST ] = value;
                else if ( tag == "offset" )
                    offset = value.toInt();
            }
            else
            {
                // Seems like an LRC file without header
                header = false;
            }
        }

        // We may fall-through, so no else
        if ( !header )
        {
            // If this is an empty line, add '\n' to the last entry if we have it
            if ( line.isEmpty() )
            {
                if ( !templyrics.isEmpty() )
                {
                    while ( templyrics.contains( last_time ) )
                        last_time++;

                    templyrics.insert( last_time, "[BR]" );
                    last_time++;
                }

                continue;
            }

            // Here we have one of three scenarios:
            // [time]text (could be lrcv1 or v2)
            // [time]text[time]text (lrcv2); or
            // [time][time]text (lrcv1)
            QRegExp regex( "\\[([\\d:\\.]+)\\]" );
            int pos = regex.indexIn( line );

            if ( pos != 0 )
                throw("Invalid LRC file, time doesn't start at pos 0");

            last_time = parseLRCTime( regex.cap( 1 ) );

            // We got the first timing, now see if it is followed by another time mark immediately
            pos = regex.matchedLength();

            if ( pos >= line.length() )
                throw("Invalid LRC file, invalid time");

            if ( line[pos] == '[' )
            {
                // This is LRCv1; collect all timings now
                QList<qint64> v1timings;
                v1timings.push_back( last_time );

                while ( pos + 1 >= line.length() && line[pos + 1] == '[' )
                {
                    int npos = regex.indexIn( line, pos + 1 );

                    if ( npos != pos + 1 )
                        throw("Invalid LRC file, neither V2 nor V1");

                    qint64 time = parseLRCTime( regex.cap( 1 ) );
                    v1timings.push_back( time );
                    pos = npos;
                }

                // Now we're on the lyrics line; add it with appropriate timing
                QString text = line.mid( pos ) + "\n";

                // and move them to the global storage
                Q_FOREACH (qint64 time, v1timings )
                {
                    // If we already have a key there with such time, increase it 1ms
                    if ( templyrics.contains( time ) )
                        templyrics[time].append( text );
                    else
                        templyrics.insert( time, text );
                }
            }
            else
            {
                // This is not LRCv1, so split the line on [ symbol, so each array
                // element contains both the string and the timing
                QStringList elements = line.split( '[' );

                // First element is empty as the line starts with [
                elements.removeFirst();

                Q_FOREACH( QString element, elements )
                {
                    // An empty element
                    // Here we got something like 00:11:2] text more text
                    // anything below ] is time, after - text
                    int idx = element.indexOf( ']' );

                    if ( idx == -1 )
                        throw ("Invalid LRC timing mark");

                    last_time = parseLRCTime( element.left( idx ) );

                    // NOTE1:
                    // Time tags following each other mean end times, i.e. [0:02.1]Say [0:02:2][0:02.3]please[0:02.6]
                    // See NOTE2
                    QString text;

                    if ( idx + 1 < element.length() )
                    {
                        text = element.mid( idx + 1 );

                        // If we already have text starting at that time, just append current text
                        if ( templyrics.contains( last_time ) )
                            templyrics[last_time].append( text );
                        else
                            templyrics.insert( last_time,  text );
                    }
                    else
                    {
                        // Appending makes no sense; make sure our time is unique
                        while ( templyrics.contains( last_time ) )
                            last_time++;

                        templyrics.insert( last_time, "" );
                    }
                }

                while ( templyrics.contains( last_time ) )
                    last_time++;

                templyrics.insert( last_time, "[BR]" );
            }
        }
    }

    // Now convert the lyrics from map to output data
    for ( QMap<qint64, QString>::const_iterator it = templyrics.begin(); it != templyrics.end(); ++it )
    {
        // NOTE2: Empty text value means the timing tag is closing one, so we just apply it as duration. See NOTE1.
        if ( it.value().isEmpty() && !output.isEmpty() )
            output.last().duration = it.key() - output.last().start;
        else if ( it.value() == "[BR]" )
            output.push_back( Lyric( it.key(), "" ));
        else
            output.push_back( Lyric( it.key(), it.value() ));
    }
}

qint64 LyricsParser_LRC::parseLRCTime(const QString &str)
{
    qint64 time;

    // Could be either 02:23 or 02:44.04
    int minsep = str.indexOf( ':' );

    if ( minsep == -1 )
        throw("Cannot parse time: no min separator");

    // Do we also have the milliseconds?
    int msecsep = str.indexOf( '.' );

    if ( msecsep != -1 )
    {
        time = str.left( minsep ).toInt() * 60000
                + str.mid( minsep + 1, msecsep - minsep - 1 ).toInt() * 1000
                + str.mid( msecsep + 1 ).toInt() * 10;
    }
    else
    {
        time = str.left( minsep ).toInt() * 60000
                + str.mid( minsep + 1 ).toInt() * 1000;
    }

    time += offset;
    return time;
}
