#include <QImage>
#include <QPainter>
#include <QFontMetrics>

#include "playerlyricstext.h"
#include "settings.h"
#include "libkaraokelyrics/lyricsloader.h"


PlayerLyricsText::PlayerLyricsText()
{
    m_nextUpdateTime = 0;
    m_currentLine = 0;
    m_longestLine = 0;

    m_renderFont = Settings::g()->playerLyricsFont;
}

bool PlayerLyricsText::load( QIODevice &file, const QString& filename )
{
    LyricsLoader::Container lyrics;
    LyricsLoader::Properties properties;

    LyricsLoader loader( properties, lyrics );

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

    m_nextUpdateTime = m_lines.first().startTime();
    m_showTitleTime = 0;

    //TODO: the rest of metadata
    if ( properties.contains( LyricsLoader::PROP_ARTIST ) && properties.contains( LyricsLoader::PROP_TITLE ) )
    {
        m_artist = properties[ LyricsLoader::PROP_ARTIST ];
        m_title = properties[ LyricsLoader::PROP_TITLE ];

        if ( firstLyricStart() > MIN_INTERVAL_AFTER_TITLE + MIN_DURATION_TITLE )
        {
            m_nextUpdateTime = 0;
            m_showTitleTime = qMin( firstLyricStart() - MIN_INTERVAL_AFTER_TITLE, (qint64) MAX_DURATION_TITLE );
        }
    }


    //foreach ( const PlayerLyricTextLine& t, m_lines )
    //    t.dump();

    //qDebug("Longest line for rendering is %d", m_longestLine );
    //m_lines[m_longestLine].dump();

    return true;
}

qint64 PlayerLyricsText::nextUpdate() const
{
    return m_nextUpdateTime;
}

int PlayerLyricsText::largetsFontSize( const QSize& size, const QString& text )
{
    int maxsize = 128;
    int minsize = 8;
    int cursize;
    QFont testfont( Settings::g()->playerLyricsFont );

    // We are trying to find the maximum font size which fits by doing the binary search
    while ( maxsize - minsize > 1 )
    {
        cursize = minsize + (maxsize - minsize) / 2;
        testfont.setPointSize( cursize );
        QFontMetrics fm( testfont );
        //qDebug("%d-%d: trying font size %d to draw on %d: width %d", minsize, maxsize, cursize, drawing_width, fm.width(m_longestLyricLine) );

        if ( fm.width( text )< size.width() )
            minsize = cursize;
        else
            maxsize = cursize;
    }

    return cursize;
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
        testfont.setPointSize( cursize );
        QFontMetrics fm( testfont );
        //qDebug("%d-%d: trying font size %d to draw on %d: width %d", minsize, maxsize, cursize, drawing_width, fm.width(m_longestLyricLine) );

        if ( m_lines[m_longestLine].screenWidth( fm ) < drawing_width  )
            minsize = cursize;
        else
            maxsize = cursize;
    }

    //qDebug("Chosen minimum size: %d", minsize );
    m_renderFont.setPointSize( minsize );
}

void PlayerLyricsText::renderTitle(qint64 timeleft, QImage &image)
{
    int drawing_width = image.size().width() - (image.size().width() * PADDING_LEFTRIGHT_PERCENTAGE * 2) / 100;
    QSize size( drawing_width, image.height() );

    int fs_artist = largetsFontSize( size, m_artist );
    int fs_title = largetsFontSize( size, m_title );

    QPainter p( &image );

    // Use the smallest size so we can fit for sure
    m_renderFont.setPointSize( qMin( fs_artist, fs_title ) );
    p.setFont( m_renderFont );

    QColor color( Qt::white );

    if ( timeleft < TITLE_FADEOUT_TIME )
    {
        color.setAlpha( timeleft * 255 / TITLE_FADEOUT_TIME );
        m_nextUpdateTime = 0;
    }

    PlayerLyricTextLine::drawOutlineText( p, image.size(), 10, color, m_artist );
    PlayerLyricTextLine::drawOutlineText( p, image.size(), 60, color, m_title );
}

qint64 PlayerLyricsText::firstLyricStart() const
{
    return m_lines.first().startTime();
}

void PlayerLyricsText::drawNotification(QPainter &p, qint64 timeleft)
{
    int drawing_width = m_usedImageSize.width() - (m_usedImageSize.width() * PADDING_LEFTRIGHT_PERCENTAGE * 2) / 100;

    p.setBrush( Qt::white );
    p.drawRect( PADDING_LEFTRIGHT_PERCENTAGE, 20, timeleft * drawing_width / MAX_NOTIFICATION_DURATION, 40 );
}


bool PlayerLyricsText::render(qint64 timems, QImage &image)
{
    // Title time?
    if ( m_showTitleTime > 0 && timems < m_showTitleTime )
    {
        renderTitle( m_showTitleTime - timems, image );
        return true;
    }

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
    else
    {
        // First line, so we almost always draw the notification when it is time
        if ( timems < firstLyricStart() && firstLyricStart() - timems <= MAX_NOTIFICATION_DURATION )
            drawNotification( p, firstLyricStart() - timems );
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
