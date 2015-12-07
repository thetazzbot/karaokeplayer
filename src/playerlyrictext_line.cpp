#include "libkaraokelyrics/lyricsparser.h"
#include "playerlyrictext_line.h"

PlayerLyricTextLine::PlayerLyricTextLine()
{

}

PlayerLyricTextLine::PlayerLyricTextLine(const LyricsLoader::Container &line)
{
    m_line = line;
}

qint64 PlayerLyricTextLine::startTime() const
{
    if ( m_line.isEmpty() )
        return -1;

    return m_line[0].start;
}

qint64 PlayerLyricTextLine::draw(qint64 time, int yoffset, const QFont &font, QImage &image)
{

}

int PlayerLyricTextLine::screenWidth(const QFontMetrics &font)
{
    QString text;

    for ( int i = 0; i < m_line.size(); i++ )
    {
        text.append( m_line[i].text );
    }

    return font.boundingRect( text ).width();
}


void PlayerLyricTextLine::dump(qint64) const
{
    for ( int i = 0; i < m_line.size(); i++ )
    {
        qDebug("%s%s - %s: '%s' ", i == 0 ? "" : "    ",
               qPrintable( LyricsParser::timeAsText( m_line[i].start) ),
               m_line[i].duration == -1 ? "?" : qPrintable( LyricsParser::timeAsText( m_line[i].start + m_line[i].duration ) ),
               qPrintable( m_line[i].text ) );
    }
}
