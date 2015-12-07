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
        qDebug("i %d, %s", i, qPrintable(lyrics[i].text));
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

    foreach ( const PlayerLyricTextLine& t, m_lines )
        t.dump();

    qDebug("Longest line for rendering is %d", m_longestLine );
    m_lines[m_longestLine].dump();

    return true;
}

qint64 PlayerLyricsText::nextUpdate() const
{
    return m_nextLyricTime;
}

bool PlayerLyricsText::render(qint64 timems, QImage &image)
{
    if ( image.size() != m_usedImageSize )
    {
        m_usedImageSize = image.size();
        calculateFontSize();
    }
/*
    // FIXME: end animation
    if ( m_currentSentence >= m_lyrics.size() )
        return true;

    // Draw until we have screen space
    QPainter p( &image );
    QFontMetrics fm( m_renderFont );

    int yoffset = (m_usedImageSize.height() * PADDING_TOPBOTTOM_PERCENTAGE) / 100;
    int ybottom = m_usedImageSize.height() - yoffset - fm.height();
    int lyric = m_currentLyric;

    while ( yoffset > ybottom )
    {
        yoffset += ybottom;
    }

*/

    return true;
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
