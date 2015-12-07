#include <QRegExp>
#include <QStringList>

#include "lyricsparser_powerkaraoke.h"

LyricsParser_PowerKaraoke::LyricsParser_PowerKaraoke()
{

}

static int powerKaraokeTime( QString time )
{
    int timing = 0;

    if ( time.contains(":") )
    {
        QStringList parts = time.split( ":" );
        timing = parts[0].toInt() * 60000;
        time = parts[1];
    }

    timing += (int) (time.toFloat() * 1000 );
    return timing;
}


void LyricsParser_PowerKaraoke::parse(QIODevice &file, LyricsLoader::Container &output, LyricsLoader::Properties &properties)
{
    QStringList text = loadText( file );

    // For the PowerKaraoke format there is no header, just times.
    QRegExp regex("^([0-9.:]+) ([0-9.:]+) (.*)");

    // Analyze each line
    for ( int i = 0; i < text.size(); i++ )
    {
        QString line = text[i];

        if ( line.isEmpty() )
            continue;

        // Try to match the sync first
        if ( line.indexOf( regex ) == -1 )
            throw("Not a valid PowerKaraoke lyric file");

        int start = powerKaraokeTime( regex.cap( 1 ) );
        //int end = powerKaraokeTime( regex.cap( 2 ) );
        QString text = regex.cap( 3 ).trimmed();

        if ( text.endsWith( "\\n" ) )
        {
            // Chop last two, and add \n
            text.chop( 2 );
            text += "\n";
        }
        else if ( !text.endsWith( "-" ) )
        {
            text += " ";
        }
        else
        {
            text.chop( 1 );
        }

        output.push_back( Lyric( start, text ) );
    }
}
