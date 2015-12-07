#include <QFile>

#include "lyricsloader.h"
#include "lyricsparser.h"
#include "lyricsparser_midi.h"
#include "lyricsparser_kfn.h"
#include "lyricsparser_kok.h"
#include "lyricsparser_lrc.h"
#include "lyricsparser_texts.h"


Lyric::Lyric(qint64 start_, const QString &text_, qint64 duration_, int pitch_)
{
    start = start_;
    text = text_;
    duration = duration_;
    pitch = pitch_;
}

LyricsLoader::LyricsLoader(Properties &properties, Container &container)
    : m_lyricsOutput(container), m_props(properties)
{
}

bool LyricsLoader::parse(const QString &lyricsfile, QIODevice &file)
{
    try
    {
        LyricsParser * parser = 0;

        if ( lyricsfile.endsWith( ".mid", Qt::CaseInsensitive)
        || lyricsfile.endsWith( ".midi", Qt::CaseInsensitive)
        || lyricsfile.endsWith( ".kar", Qt::CaseInsensitive) )
        {
            parser = new LyricsParser_MIDI();
        }
        else if ( lyricsfile.endsWith( ".lrc", Qt::CaseInsensitive) )
            parser = new LyricsParser_LRC();
        else if ( lyricsfile.endsWith( ".kok", Qt::CaseInsensitive) )
            parser = new LyricsParser_KOK();
        else if ( lyricsfile.endsWith( ".txt", Qt::CaseInsensitive) )
        {
            parser = new LyricsParser_Texts();
        }
        else
        {
            m_errorMsg = "No parser for this format";
            return false;
        }

        parser->parse( file, m_lyricsOutput, m_props );
        delete parser;

        calculateDurations();
        return true;
    }
    catch ( const char * err )
    {
        m_errorMsg = err;
    }
    catch ( const QString& err )
    {
        m_errorMsg = err;
    }

    return false;
}

bool LyricsLoader::isSupportedFile(const QString &file)
{
    static const char * extlist[] = { ".mid", ".midi", ".kar", ".txt", ".lrc", ".kok", ".kfn", 0 };

    for ( const char * ext = extlist[0]; *ext; ext += strlen(ext) + 1 )
        if ( file.endsWith( ext, Qt::CaseInsensitive ) )
            return true;

    return false;
}

void LyricsLoader::calculateDurations()
{
    for ( int i = 0; i < m_lyricsOutput.size(); i++ )
    {
        // Keep the duration if already present
        if ( m_lyricsOutput[i].duration != -1 )
            continue;

        // Do we have next lyrics
        // If not, calculate it as min( time to next, 500ms)
        if ( i < m_lyricsOutput.size() - 1 )
            m_lyricsOutput[i].duration = qMin( m_lyricsOutput[i+1].start - m_lyricsOutput[i].start, (qint64) 500 );
        else
            m_lyricsOutput[i].duration = 500;
    }
}

void LyricsLoader::dump(const Container &output)
{
    Q_FOREACH ( const Lyric& l, output )
    {
        if ( l.text.isEmpty() )
            printf("\n");
        else
            printf("%s - %s: '%s' ", qPrintable( LyricsParser::timeAsText( l.start) ),
                   l.duration == -1 ? "?" : qPrintable( LyricsParser::timeAsText( l.start + l.duration ) ),
                   qPrintable( l.text ) );
    }

    printf("\n");
}
