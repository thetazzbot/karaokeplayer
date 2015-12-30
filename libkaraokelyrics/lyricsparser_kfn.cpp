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

#include <QStringList>
#include <QRegExp>

#include "lyricsparser_kfn.h"

LyricsParser_KFN::LyricsParser_KFN( ConvertEncoding * converter )
    : LyricsParser( converter )
{
}

void LyricsParser_KFN::parse(QIODevice * file, LyricsLoader::Container &output, LyricsLoader::Properties &properties)
{
    QString songini = loadText( file ).join( '\n' );
    songini.replace( QRegExp("[\r\n]+"), "\n" );
    QStringList lines = songini.split( "\n" );

    // Parse the song.ini and fill up the sync and text arrays
    QStringList texts;
    QList< int > syncs;

    QRegExp patternSync( "^Sync[0-9]+=(.+)" );
    QRegExp patternText( "^Text[0-9]+=(.*)" );

    // Analyze each line
    for ( int i = 0; i < lines.size(); i++ )
    {
        QString line = lines[i];

        // Try to match the sync first
        if ( line.indexOf( patternSync ) != -1 )
        {
            // Syncs are split by comma
            QStringList values = patternSync.cap( 1 ).split(",");

            for ( int v = 0; v < values.size(); v++ )
                syncs.push_back( values[v].toInt() );
        }

        // Now the text
        if ( line.indexOf( patternText ) != -1 )
        {
            if ( !patternText.cap( 1 ).isEmpty() )
            {
                // Text is split by word and optionally by the slash
                QStringList values = patternText.cap( 1 ).split(" ");

                for ( int v = 0; v < values.size(); v++ )
                {
                    QStringList morevalues = values[v].split( "/" );

                    for ( int vv = 0; vv < morevalues.size(); vv++ )
                        texts.push_back( morevalues[vv] );

                    // We split by space, so add it at the end of each word
                    texts.last() = texts.last() + " ";
                }
            }

            // Line matched, so make it a line
            if ( texts.size() > 2 && texts[ texts.size() - 2 ] != "\n" )
                texts.push_back( "\n" );
        }
    }

    int curr_sync = 0;
    bool has_linefeed = false;
    int lines_no_block = 0;
    int lastsync = -1;

    // The original timing marks are not necessarily sorted, so we add them into a map
    // and then output them from that map
    QMap< int, QString > sortedLyrics;

    for ( int i = 0; i < texts.size(); i++ )
    {
        if ( texts[i] == "\n" )
        {
            if ( lastsync == -1 )
                continue;

            if ( has_linefeed )
                lines_no_block = 0;
            else if ( ++lines_no_block > 6 )
            {
                lines_no_block = 0;
                sortedLyrics[ lastsync ] += "\n";
            }

            has_linefeed = true;
            sortedLyrics[ lastsync ] += "\n";
            continue;
        }
        else
            has_linefeed = false;

        // Get the time if we have it
        if ( curr_sync >= syncs.size() )
            continue;

        lastsync = syncs[ curr_sync++ ];
        sortedLyrics.insert( lastsync, texts[i] );
    }

    for ( QMap< int, QString >::const_iterator it = sortedLyrics.begin(); it != sortedLyrics.end(); ++it )
    {
        qint64 timing = it.key() * 10;
        Lyric lyr( timing, it.value() );

        // Last line?
        if ( lyr.text.endsWith( '\n') )
        {
            // Remove the \n
            lyr.text.chop( 1 );
            output.push_back( lyr );

            // and add an empty lyric
            output.push_back( Lyric( timing ) );
        }
        else
        {
            output.push_back( lyr );
        }
    }
}
