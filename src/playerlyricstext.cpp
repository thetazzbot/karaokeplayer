#include <QImage>
#include <QPainter>
#include <QFontMetrics>

#include "playerlyricstext.h"
#include "settings.h"
#include "karaokepainter.h"
#include "libkaraokelyrics/lyricsloader.h"


PlayerLyricsText::PlayerLyricsText()
{
    m_nextUpdateTime = 0;
    m_currentLine = 0;
    m_longestLine = 0;

    m_renderFont = pSettings->playerLyricsFont;
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
            if ( m_lines.isEmpty() && linestartidx == -1 )
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
//        t.dump();

    //qDebug("Longest line for rendering is %d", m_longestLine );
    //m_lines[m_longestLine].dump();

    return true;
}

qint64 PlayerLyricsText::nextUpdate() const
{
    return m_nextUpdateTime;
}


void PlayerLyricsText::calculateFontSize()
{
    int fontsize = KaraokePainter::largetsFontSize( pSettings->playerLyricsFont,
                                                    m_usedImageSize.width(),
                                                    m_lines[m_longestLine].fullLine() );

    //qDebug("Chosen minimum size: %d", fontsize );
    m_renderFont.setPointSize( fontsize );
}

void PlayerLyricsText::renderTitle( KaraokePainter &p )
{
    int fs_artist = p.largetsFontSize( m_artist );
    int fs_title = p.largetsFontSize( m_title );

    // Use the smallest size so we can fit for sure
    m_renderFont.setPointSize( qMin( fs_artist, fs_title ) );
    p.setFont( m_renderFont );

    QColor color( Qt::white );

    if ( m_showTitleTime - p.time() < TITLE_FADEOUT_TIME )
    {
        color.setAlpha( (m_showTitleTime - p.time() ) * 255 / TITLE_FADEOUT_TIME );
        m_nextUpdateTime = 0;
    }

    p.drawCenteredOutlineText( 10, color, m_artist );
    p.drawCenteredOutlineText( 60, color, m_title );
}

qint64 PlayerLyricsText::firstLyricStart() const
{
    return m_lines.first().startTime();
}

void PlayerLyricsText::drawNotification(KaraokePainter &p, qint64 timeleft)
{
    p.setPen( Qt::black );
    p.setBrush( Qt::white );
    p.drawRect( 0, 0, timeleft * p.rect().width() / MAX_NOTIFICATION_DURATION, 10 );
}


bool PlayerLyricsText::render(KaraokePainter &p)
{
    qint64 timems = p.time();

    p.setClipAreaText();

    // Title time?
    if ( m_showTitleTime > 0 && timems < m_showTitleTime )
    {
        renderTitle( p );
        return true;
    }

    if ( p.rect() != m_usedImageSize )
    {
        m_usedImageSize = p.rect();
        calculateFontSize();
    }

    int current;

    // Current lyric lookup:
    for ( current = 0; current < m_lines.size(); current++ )
    {
        // For empty lines we look up for the next time
        if ( m_lines[current].isEmpty() )
            continue;

        // Line ended already
        if ( m_lines[current].endTime() < p.time() )
            continue;

        // We found it
        break;
    }

    // Draw until we have screen space
    QFontMetrics fm( m_renderFont );
    p.setFont( m_renderFont );

    // Draw current line(s)
    int yoffset = 0;
    int ybottom = p.rect().height() - yoffset;
    yoffset += + fm.height();

    int scroll_passed = 0, scroll_total = 0;

    // If we're playing after the first line, start scrolling it so for the moment the next line it is due
    // it would be at the position of the second line.
    if ( current > 0 && current < m_lines.size() )
    {
        // Should we scroll?
        if ( p.time() > m_lines[current].startTime() )
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

    if ( current == m_lines.size() )
        return true;

    // If a current line is empty, track down once
    if ( m_lines[current].isEmpty() && current > 0 )
        current--;

    while ( yoffset < ybottom && current < m_lines.size() )
    {
        if ( !m_lines[current].isEmpty() )
        {
            if ( scroll_total > 0 )
            {
                // Animate out the first line
                int percentage = (100 * scroll_passed) / scroll_total;
                m_lines[current].drawDisappear( p, percentage, yoffset );
                scroll_total = 0;
            }
            else
            {
                m_lines[current].draw( p, yoffset );
            }
        }

        current++;
        yoffset += fm.height();
    }

    return true;
}
