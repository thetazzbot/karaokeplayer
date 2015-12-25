#include "lyricsparser_kok.h"

LyricsParser_KOK::LyricsParser_KOK()
{
}

void LyricsParser_KOK::parse(QIODevice *file, LyricsLoader::Container &output, LyricsLoader::Properties &)
{
    QStringList lyrics = loadText( file );

    // J'ai ;7,9050634;tra;8,0651993;vail;8,2144914;lé;8,3789922;
    Q_FOREACH ( QString line, lyrics )
    {
        if ( line.isEmpty() )
            continue;

        QStringList entries = line.split( ";" );

        // Must be even
        if ( (entries.size() % 2) != 0 )
            throw("Invalid number of entries");

        for ( int i = 0; i < entries.size() / 2; i++ )
        {
            QString text = entries[2 * i];
            QString timing = entries[2 * i + 1].replace( ",", "." );

            int timevalue = (int) ( timing.toFloat() * 1000 );
            if ( i == entries.size() / 2 - 1 )
                text.append( '\n' );

            output.push_back( Lyric( timevalue, text ) );
        }
    }
}

