#include "libkaraokelyrics/lyricsparser.h"
#include "playerlyrictext_line.h"
#include "settings.h"
#include "karaokepainter.h"

PlayerLyricTextLine::PlayerLyricTextLine()
{

}

PlayerLyricTextLine::PlayerLyricTextLine(const LyricsLoader::Container &line)
{
    if ( pSettings->playerLyricsTextEachCharacter )
    {
        // Split it per-character
        foreach ( Lyric l, line )
        {
            int perchar = l.duration / l.text.length();

            for ( int i = 0; i < l.text.length(); i++ )
                m_line.push_back( Lyric( l.start + perchar * i, QString(l.text[i]), perchar, l.pitch ) );
        }
    }
    else
        m_line = line;

    foreach ( Lyric l, line )
        m_text += l.text;
}

qint64 PlayerLyricTextLine::startTime() const
{
    if ( m_line.isEmpty() )
        return -1;

    return m_line.first().start;
}

qint64 PlayerLyricTextLine::endTime() const
{
    if ( m_line.isEmpty() )
        return -1;

    return m_line.last().start + m_line.last().duration;
}

qint64 PlayerLyricTextLine::draw( KaraokePainter& p, qint64 time, int yoffset )
{
    if ( m_line.isEmpty() )
        return 0;

    int x = (p.textRect().width() - p.fontMetrics().boundingRect( m_text ).width()) / 2 + p.textRect().x();
\
    // Draw the text string
    for ( int i = 0; i < m_line.size(); i++ )
    {
        p.drawOutlineText( x,
                           yoffset,
                           m_line[i].start < time ? pSettings->playerLyricsTextAfterColor
                                                : pSettings->playerLyricsTextBeforeColor,
                           m_line[i].text );

        x += p.fontMetrics().width( m_line[i].text );
    }

    return 0;
}

void PlayerLyricTextLine::drawDisappear(KaraokePainter& p, int percentage, int yoffset)
{
    if ( m_line.isEmpty() )
        return;

    float scale_level = 1.0F - ((float) percentage / 100.0F);

    p.save();
    p.scale( scale_level, scale_level );

    int x = (p.textRect().width() - p.fontMetrics().boundingRect( m_text ).width()) / 2 + p.textRect().x();

    // Draw the text string
    for ( int i = 0; i < m_line.size(); i++ )
    {
        p.drawOutlineText( x, yoffset, pSettings->playerLyricsTextAfterColor, m_line[i].text );
        x += p.fontMetrics().width( m_line[i].text );
    }

    p.restore();
}

int PlayerLyricTextLine::screenWidth(const QFontMetrics &font)
{
    return font.boundingRect( m_text ).width();
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
