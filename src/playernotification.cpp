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

#include "settings.h"
#include "currentstate.h"
#include "songqueue.h"
#include "playernotification.h"

PlayerNotification * pNotification;

PlayerNotification::PlayerNotification(QObject *parent)
    : QObject( parent )
{
    m_scrollStep = 3;
    m_lastScreenHeight = 0;
    m_notificationLine = m_firstItem = tr("Queue is empty");

    if ( !pSettings->customBackground.isEmpty() )
        m_customBackground.load( pSettings->customBackground );

    reset();
}

void PlayerNotification::showStopped()
{
    m_customMessage = pCurrentState->webserverURL.isEmpty() ? "Please select a song" : pCurrentState->webserverURL;
}

qint64 PlayerNotification::drawTop( KaraokePainter &p )
{
    // If the height change, see how large the font we could fit height-wise
    if ( m_lastScreenHeight != p.notificationRect().height() )
    {
        m_lastScreenHeight = p.notificationRect().height();
        p.tallestFontSize( m_font, m_lastScreenHeight );
    }

    p.setPen( Qt::white );
    p.setFont( m_font );

    int remainingms = -1;

    if ( pCurrentState->playerState != CurrentState::PLAYERSTATE_STOPPED )
        remainingms = qMax( 0, pCurrentState->playerDuration - pCurrentState->playerPosition );

    if ( remainingms == -1 || remainingms > 5000 )
    {
        if ( m_scrollOffset >= p.fontMetrics().width( m_notificationLine[m_textOffset] ) )
        {
            m_scrollOffset = 0;
            m_textOffset++;

            if ( m_textOffset >= m_notificationLine.length() )
                m_textOffset = 0;
        }
        else
            m_scrollOffset++;

        p.drawText( -m_scrollOffset, p.fontMetrics().ascent(), m_notificationLine.mid( m_textOffset ) + "      " + m_notificationLine );
    }
    else
        p.drawText( 0, p.fontMetrics().ascent(), m_firstItem );

    return 0;
}

qint64 PlayerNotification::drawRegular(KaraokePainter &p)
{
    if ( !m_customMessage.isEmpty() )
    {
        if ( !m_customBackground.isNull() )
            p.drawImage( p.rect(), m_customBackground, m_customBackground.rect() );

        m_customFont.setPointSize( p.largestFontSize( m_customFont, p.textRect().width(), m_customMessage ));

        p.setFont( m_customFont );
        p.drawCenteredOutlineText( 50, Qt::white, m_customMessage );
    }

    return 0;
}

void PlayerNotification::queueUpdated()
{
    QList<SongQueue::Song> queue;

    // Get the song list
    pSongQueue->exportQueue( queue );

    m_notificationLine.clear();
    m_firstItem.clear();
    m_textQueueSize.clear();

    if ( !queue.isEmpty() )
    {
        m_notificationLine.clear();
        int elements = 0;

        for ( int i = 0; i < queue.size(); i++ )
        {
            // Do not show the song if it is being played
            if ( queue[i].state == SongQueue::Song::STATE_PLAYING )
                continue;

            if ( elements == 0 )
                m_firstItem = tr("Next: %1 (%2)") .arg( queue[i].title ) .arg( queue[i].singer );

            m_notificationLine.append( tr("%1: %2 (%3) ").arg( elements + 1 ) .arg( queue[i].title ) .arg( queue[i].singer ) );
            elements++;
        }

        m_textQueueSize = tr("[%1]").arg( elements );
    }

    reset();
}

void PlayerNotification::setOnScreenMessage(const QString &message)
{
    QMutexLocker m( &m_mutex );
    m_customMessage = message;
}

void PlayerNotification::clearOnScreenMessage()
{
    QMutexLocker m( &m_mutex );
    m_customMessage.clear();
}

void PlayerNotification::setMessage(const QString &message, int show)
{

}

void PlayerNotification::reset()
{
    m_textOffset = 0;
    m_scrollOffset = 0;
}
