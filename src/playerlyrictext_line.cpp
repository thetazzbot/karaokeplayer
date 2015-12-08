#include "libkaraokelyrics/lyricsparser.h"
#include "playerlyrictext_line.h"
#include "settings.h"

PlayerLyricTextLine::PlayerLyricTextLine()
{

}

PlayerLyricTextLine::PlayerLyricTextLine(const LyricsLoader::Container &line)
{
    if ( Settings::g()->playerLyricsTextEachCharacter )
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

qint64 PlayerLyricTextLine::draw(qint64 time, int width, int yoffset, QPainter &p)
{
    if ( m_line.isEmpty() )
        return 0;

    int x = (width - p.fontMetrics().boundingRect( m_text ).width()) / 2;
\
    // Draw the text string
    for ( int i = 0; i < m_line.size(); i++ )
    {
        drawOutlineText( p, x, yoffset,
                         m_line[i].start < time ? Settings::g()->playerLyricsTextAfterColor
                                                : Settings::g()->playerLyricsTextBeforeColor,
                         m_line[i].text );

        x += p.fontMetrics().width( m_line[i].text );
    }

    return 0;
}

void PlayerLyricTextLine::drawDisappear(int percentage, int width, int yoffset, QPainter &p)
{
    if ( m_line.isEmpty() )
        return;

    float scale_level = 1.0F - ((float) percentage / 100.0F);

    p.save();
    p.scale( scale_level, scale_level );

    int x = (width - p.fontMetrics().boundingRect( m_text ).width()) / 2;

    // Draw the text string
    for ( int i = 0; i < m_line.size(); i++ )
    {
        drawOutlineText( p, x, yoffset, Settings::g()->playerLyricsTextAfterColor, m_line[i].text );
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

void PlayerLyricTextLine::drawOutlineText(QPainter &p, int x, int y, const QColor &color, const QString &text)
{
    p.setPen( Qt::black );
    p.drawText( x - 1, y - 1, text );
    p.drawText( x + 1, y - 1, text );
    p.drawText( x - 1, y + 1, text );
    p.drawText( x + 1, y + 1, text );

    p.setPen( color );
    p.drawText( x, y, text );
}

void PlayerLyricTextLine::drawOutlineText(QPainter &p, const QSize &size, int ypercentage, const QColor &color, const QString &text)
{
    int x = (size.width() - p.fontMetrics().width( text ) ) / 2;
    int y = (size.height() * ypercentage / 100 ) + p.fontMetrics().height();

    drawOutlineText( p, x, y, color, text );
}
