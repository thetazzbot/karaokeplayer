/**************************************************************************
 *  Ulduzsoft Karaoke PLayer - cross-platform desktop karaoke player      *
 *  Copyright (C) 2015-2016 George Yunaev, support@ulduzsoft.com          *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *																	      *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#include <QImage>
#include <QPainter>
#include <QFontMetrics>

#include "playerlyricstext.h"
#include "settings.h"
#include "karaokepainter.h"
#include "libkaraokelyrics/lyricsloader.h"


PlayerLyricsText::PlayerLyricsText( const QString& artist, const QString& title )
    : PlayerLyrics()
{
    m_nextUpdateTime = 0;
    m_currentLine = 0;
    m_longestLine = 0;
    m_artist = artist;
    m_title = title;

    m_renderFont = pSettings->playerLyricsFont;
}

bool PlayerLyricsText::load(QIODevice * file, const QString& filename )
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
    if ( m_properties.contains( LyricsLoader::PROP_ARTIST ) && m_properties.contains( LyricsLoader::PROP_TITLE ) )
    {
        m_artist = m_properties[ LyricsLoader::PROP_ARTIST ];
        m_title = m_properties[ LyricsLoader::PROP_TITLE ];

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
    int fontsize = KaraokePainter::largestFontSize( pSettings->playerLyricsFont,
                                                    pSettings->playerLyricsFontMaxSize,
                                                    m_usedImageSize.width(),
                                                    m_lines[m_longestLine].fullLine() );

    //qDebug("Chosen minimum size: %d", fontsize );
    m_renderFont.setPointSize( fontsize );
}

void PlayerLyricsText::renderTitle(KaraokePainter &p , qint64 time )
{
    int fs_artist = p.largestFontSize( m_artist );
    int fs_title = p.largestFontSize( m_title );

    // Use the smallest size so we can fit for sure
    m_renderFont.setPointSize( qMin( fs_artist, fs_title ) );
    p.setFont( m_renderFont );

    QColor color( Qt::white );

    if ( m_showTitleTime - time < TITLE_FADEOUT_TIME )
    {
        color.setAlpha( (m_showTitleTime - time ) * 255 / TITLE_FADEOUT_TIME );
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
    p.drawRect( p.textRect().x(), p.textRect().y(), timeleft * p.textRect().width() / MAX_NOTIFICATION_DURATION, qMax( p.textRect().height() / 50, 10 ) );
}


bool PlayerLyricsText::render(KaraokePainter &p, qint64 timems)
{
    // Title time?
    if ( m_showTitleTime > 0 && timems < m_showTitleTime )
    {
        renderTitle( p, timems );
        return true;
    }

    if ( p.textRect() != m_usedImageSize )
    {
        m_usedImageSize = p.textRect();
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
        if ( m_lines[current].endTime() < timems )
            continue;

        // We found it
        break;
    }

    // Draw until we have screen space
    QFontMetrics fm( m_renderFont );
    p.setFont( m_renderFont );

    // Draw current line(s)
    int yoffset = p.textRect().y();
    int ybottom = p.textRect().height() - yoffset;
    yoffset += + fm.height();

    int scroll_passed = 0, scroll_total = 0;

    // If we're playing after the first line, start scrolling it so for the moment the next line it is due
    // it would be at the position of the second line.
    if ( current > 0 && current < m_lines.size() )
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
                m_lines[current].draw( p, timems, yoffset );
            }
        }

        current++;
        yoffset += fm.height();
    }

    return true;
}
