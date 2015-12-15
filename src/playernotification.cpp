#include "songqueue.h"
#include "playernotification.h"

PlayerNotification * pNotification;

PlayerNotification::PlayerNotification(QObject *parent)
    : QObject( parent ), m_fontMetrics( m_font )
{
    m_lastScreenHeight = 0;
    m_notificationLine = m_firstItem = tr("Queue is empty");

    reset();
}

qint64 PlayerNotification::draw(KaraokePainter &p)
{
    // If the height change, see how large the font we could fit height-wise
    if ( m_lastScreenHeight != p.rect().height() )
    {
        m_lastScreenHeight = p.rect().height();
        p.tallestFontSize( m_font, m_lastScreenHeight );
        m_fontMetrics = QFontMetrics( m_font );
    }

    p.setPen( Qt::white );
    p.setFont( m_font );

    if ( p.duration() - p.time() > 5000 )
    {
        if ( m_scrollOffset >= m_fontMetrics.width( m_notificationLine[m_textOffset] ) )
        {
            m_scrollOffset = 0;
            m_textOffset++;

            if ( m_textOffset >= m_notificationLine.length() )
                m_textOffset = 0;
        }
        else
            m_scrollOffset++;

        p.drawText( -m_scrollOffset, m_fontMetrics.height(), m_notificationLine.mid( m_textOffset ) + "      " + m_notificationLine );
    }
    else
        p.drawText( 0, m_fontMetrics.height(), m_firstItem );

    return p.time();
}

void PlayerNotification::queueUpdated()
{
    QList<QString>  queue;

    // Get the song list
    pSongQueue->asList( queue );

    if ( !queue.isEmpty() )
    {
        m_notificationLine = tr("[%1] ").arg( queue.size() );

        for ( int i = 0; i < queue.size(); i++ )
        {
            // 0 - singer, 1 - title
            QStringList pair = queue[i].split( '|' );

            if ( i == 0 )
                m_firstItem = tr("Next: %1 (%2)") .arg( pair[1] ) .arg( pair[0] );

            m_notificationLine.append( tr("%1: %2 (%3) ").arg( i + 1 ) .arg( pair[1] ) .arg( pair[0] ) );
        }
    }
    else
    {
        m_notificationLine.clear();
        m_firstItem.clear();
    }

    reset();
}

void PlayerNotification::reset()
{
    m_textOffset = 0;
    m_scrollOffset = 0;
}
