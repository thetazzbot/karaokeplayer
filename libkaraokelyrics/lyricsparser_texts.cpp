#include "lyricsparser_texts.h"


LyricsParser_Texts::LyricsParser_Texts()
{
}

void LyricsParser_Texts::parse(QIODevice &file, LyricsLoader::Container &output, LyricsLoader::Properties &properties)
{
    QStringList lyrics = loadText( file );

    // TXT could be UltraStar or PowerKaraoke
    if ( lyrics.first().indexOf( QRegExp( "^#[a-zA-Z]+:\\s*.*\\s*$" ) ) != -1 )
        return parseUStar( lyrics, output, properties );
    else
        return parsePowerKaraoke( lyrics, output, properties );
}

void LyricsParser_Texts::parseUStar(const QStringList &text, LyricsLoader::Container &output, LyricsLoader::Properties &properties)
{
    bool header = true;
    bool relative = false;
    int bpm = -1, gap = -1;
    double msecs_per_beat = 0;
    int last_time_ms = 0;
    int next_time_ms = 0;
    int last_pitch = 0;

    for ( int i = 0; i < text.size(); i++ )
    {
        QString line = text[i];

        if ( header )
        {
            QRegExp regex( "^#([a-zA-Z]+):\\s*(.*)\\s*$" );

            if ( regex.indexIn( line ) != -1 )
            {
                QString tag = regex.cap( 1 );
                QString value = regex.cap( 2 );
                LyricsLoader::Property tagid = LyricsLoader::PROP_INVALID;

                if ( tag == "TITLE" )
                    tagid = LyricsLoader::PROP_TITLE;
                else if ( tag == "ARTIST" )
                    tagid = LyricsLoader::PROP_ARTIST;
                else if ( tag == "MP3FILE" )
                    tagid = LyricsLoader::PROP_MUSICFILE;
                else if ( tag == "BPM" )
                    bpm = value.toInt();
                else if ( tag == "GAP" )
                    gap = value.toInt();
                else if ( tag == "RELATIVE" )
                    relative = value.compare( "yes" );
                else if ( tag == "BACKGROUND" )
                    tagid = LyricsLoader::PROP_BACKGROUND;
                else if ( tag == "VIDEO" )
                    tagid = LyricsLoader::PROP_VIDEO;
//				else if ( tag == "VIDEOGAP" )
//              else if ( tag == "COVER" )
//				else if ( tag == "EDITION" )
//              else if ( tag == "LANGUAGE" )
//              else if ( tag == "GENRE" )

                if ( tagid != LyricsLoader::PROP_INVALID )
                    properties[ tagid ] = value;
            }
            else
            {
                // Tag not found; either header ended, or invalid file
                if ( bpm == -1 || gap == -1 )
                    throw("BPM and/or GAP is missing.");

                msecs_per_beat = (int) ((60.0 / (double) bpm / 4.0) * 1000.0);
                header = false;
            }
        }

        // We may fall-through, so no else
        if ( !header )
        {
            if ( line[0] != 'E' && line[0] != ':' && line[0] != '*' && line[0] != 'F' && line[0] != '-' )
                throw( "Not a valid format");

            // End?
            if ( line[0] == 'E' )
                break;

            QStringList parsed = line.split( QRegExp("\\s+") );

            if ( parsed.size() < 3 )
                throw("Invalid UltraStar file format");

            int timing = relative ? last_time_ms : 0;
            timing += parsed[1].toInt() * msecs_per_beat;

/*            // Should we add an empty field?
            if ( next_time_ms != 0 && timing > next_time_ms )
            {
                lyrics.curLyricSetTime( next_time_ms );
                lyrics.curLyricSetPitch( last_pitch );
                lyrics.curLyricAdd();
            }
*/

            Lyric lyr( timing, "" );
            lyr.duration = parsed[2].toInt() * msecs_per_beat;

            if ( parsed[0] == "F" || parsed[0] == "*"  || parsed[0] == ":" )
            {
                if ( parsed.size() < 5 )
                    throw ("not a valid UltraStar lyric file");

                if ( parsed[0] == "F" )
                    lyr.pitch |= Lyric::PITCH_FREESTYLE;
                else if ( parsed[0] == "*" )
                    lyr.pitch |= Lyric::PITCH_GOLDEN;
                else
                    lyr.pitch = parsed[3].toInt();

                lyr.text = parsed[4];
            }
            else if ( parsed[0] == "-" )
                lyr.text = "\n";

            output.push_back( lyr );
        }
    }
}

void LyricsParser_Texts::parsePowerKaraoke(const QStringList &text, LyricsLoader::Container &output, LyricsLoader::Properties &properties)
{
    //TODO
}
