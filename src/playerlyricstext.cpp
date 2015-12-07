#include <QImage>
#include <QPainter>
#include <QFontMetrics>

#include "playerlyricstext.h"
#include "settings.h"
#include "libkaraokelyrics/lyricsloader.h"


PlayerLyricsText::PlayerLyricsText()
{
    m_nextLyricTime = 0;
    m_currentLine = 0;
    m_longestLine = 0;

    m_renderFont = Settings::g()->playerLyricsFont;
}

bool PlayerLyricsText::load( QIODevice &file, const QString& filename )
{
    LyricsLoader::Container lyrics;
    LyricsLoader loader( m_properties, lyrics );

    if ( !loader.parse( filename, file ) )
    {
        m_errorMsg = loader.errorMsg();
        return false;
    }

    // Convert them into sentences, and find the longest lyrics line when rendered by a lyric font
    QFontMetrics fm( m_renderFont );
    int maxwidthvalue = 0;
    int linestartidx = -1;

    for ( int i = 0; i < lyrics.size(); i++ )
    {
        if ( lyrics[i].text.isEmpty() )
        {
            // If the first lyric is empty (weird), ignore it
            if ( i == 0 )
                continue;

            // If the empty lines follow, just add them as-is
            if ( linestartidx == -1  )
            {
                m_lines.push_back( PlayerLyricTextLine() );
                continue;
            }

            // Add the lyric line data
            m_lines.push_back( PlayerLyricTextLine( lyrics.mid( linestartidx, i - linestartidx ) ) );

            // And calculate the rendering width
            int render_width = m_lines.last().screenWidth( fm );

            if ( render_width > maxwidthvalue )
            {
                maxwidthvalue = render_width;
                m_longestLine = m_lines.size() - 1;
            }

            linestartidx = -1;
        }
        else
        {
            if ( linestartidx == -1 )
                linestartidx = i;
        }
    }

    // Trim the empty values at the end, if any
    while ( m_lines.last().isEmpty() )
        m_lines.takeLast();

    m_nextLyricTime = m_lines[0].startTime();

    //foreach ( const PlayerLyricTextLine& t, m_lines )
    //    t.dump();

    //qDebug("Longest line for rendering is %d", m_longestLine );
    //m_lines[m_longestLine].dump();

    return true;
}

qint64 PlayerLyricsText::nextUpdate() const
{
    return m_nextLyricTime;
}

void PlayerLyricsText::calculateFontSize()
{
    int maxsize = 128;
    int minsize = 8;
    int drawing_width = m_usedImageSize.width() - (m_usedImageSize.width() * PADDING_LEFTRIGHT_PERCENTAGE * 2) / 100;
    QFont testfont( Settings::g()->playerLyricsFont );

    // We are trying to find the maximum font size which fits by doing the binary search
    while ( maxsize - minsize > 1 )
    {
        int cursize = minsize + (maxsize - minsize) / 2;
        testfont.setPixelSize( cursize );
        QFontMetrics fm( testfont );
        //qDebug("%d-%d: trying font size %d to draw on %d: width %d", minsize, maxsize, cursize, drawing_width, fm.width(m_longestLyricLine) );

        if ( m_lines[m_longestLine].screenWidth( fm ) < drawing_width  )
            minsize = cursize;
        else
            maxsize = cursize;
    }

    qDebug("Chosen minimum size: %d", minsize );
    m_renderFont.setPixelSize( minsize );
}


bool PlayerLyricsText::render(qint64 timems, QImage &image)
{
    if ( image.size() != m_usedImageSize )
    {
        m_usedImageSize = image.size();
        calculateFontSize();
    }

    int current;

    // Current lyric lookup:
    for ( current = 0; current < m_lines.size(); current++ )
    {
        // Empty lines do not count
        if ( m_lines[current].isEmpty() )
            continue;

        // Line ended already
        if ( m_lines[current].endTime() < timems )
            continue;

        // We found it
        break;
    }

    // Draw until we have screen space
    QFontMetrics fm( m_renderFont );
    QPainter p( &image );
    p.setFont( m_renderFont );

    // Draw current line(s)
    int yoffset = (m_usedImageSize.height() * PADDING_TOPBOTTOM_PERCENTAGE) / 100;
    int ybottom = m_usedImageSize.height() - yoffset;
    yoffset += + fm.height();

    int scroll_passed = 0, scroll_total = 0;

    // If we're playing after the first line, start scrolling it so for the moment the next line it is due
    // it would be at the position of the second line.
    if ( current > 0 )
    {
        // Should we scroll?
        if ( timems > m_lines[current].startTime() )
        {
            scroll_total = m_lines[current].endTime() - m_lines[current].startTime();
            scroll_passed = timems - m_lines[current].startTime();
            yoffset -= scroll_passed * fm.height() / scroll_total;
        }

        // Draw one more line in front (so our line is always the second one)
        current--;
    }

    while ( yoffset < ybottom && current < m_lines.size() )
    {
        if ( !m_lines[current].isEmpty() )
        {
            if ( scroll_total > 0 )
            {
                // Animate out the first line
                int percentage = (100 * scroll_passed) / scroll_total;
                m_lines[current].drawDisappear( percentage, image.width(), yoffset, p );
                scroll_total = 0;
            }
            else
            {
                m_lines[current].draw( timems, image.width(), yoffset, p );
            }
        }

        current++;
        yoffset += fm.height();
    }

    // Draw disappearing lyrics
    return true;
}
