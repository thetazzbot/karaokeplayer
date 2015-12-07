#include <QImage>
#include <QPainter>
#include <QFontMetrics>

#include "playerlyricstext.h"
#include "settings.h"
#include "libkaraokelyrics/lyricsloader.h"


PlayerLyricsText::PlayerLyricsText()
{
    m_currentSentence = 0;
    m_nextLyricTime = 0;

    m_renderFont = Settings::g()->playerLyricsFont;
}

bool PlayerLyricsText::load( QIODevice &file, const QString& filename )
{
    LyricsLoader loader( m_properties, m_lyrics );

    if ( !loader.parse( filename, file ) )
    {
        m_errorMsg = loader.errorMsg();
        return false;
    }

    // Find the longest lyrics line when rendered by a lyric font
    QFontMetrics fm( m_renderFont );
    QString line;
    int maxvalue = 0;

    foreach ( const Lyric& l, m_lyrics )
    {
        if ( l.text.isEmpty() )
        {
            if ( !line.isEmpty() && fm.width( line ) > maxvalue )
            {
                maxvalue = fm.width( line );
                m_longestLyricLine = line;
            }

            line.clear();
        }
        else
            line.append( l.text );
    }

    m_nextLyricTime = m_lyrics[0].start;

    //qDebug("Longest line for rendering is '%s'", qPrintable( m_longestLyricLine ) );
    //loader.dump( m_lyrics );
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

        if ( fm.width( m_longestLyricLine ) < drawing_width  )
            minsize = cursize;
        else
            maxsize = cursize;
    }

    qDebug("Chosen minimum size: %d", minsize );
    m_renderFont.setPixelSize( minsize );
}
