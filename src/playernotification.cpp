#include "songqueue.h"
#include "playernotification.h"

PlayerNotification * pNotification;

PlayerNotification::PlayerNotification(QObject *parent)
    : QObject( parent ), m_fontMetrics( m_font )
{
    m_lastScreenHeight = 0;
    m_notificationLine = m_firstItem = tr("Queue is empty");

    m_customMessage = "Please select a song";
    reset();
}

qint64 PlayerNotification::drawTop(KaraokePainter &p, qint64 remainingms)
{
    // If the height change, see how large the font we could fit height-wise
    if ( m_lastScreenHeight != p.notificationRect().height() )
    {
        m_lastScreenHeight = p.notificationRect().height();
        m_fontMetrics = QFontMetrics( m_font );
    }

    p.setPen( Qt::white );
    p.setFont( m_font );

    if ( remainingms == -1 || remainingms > 5000 )
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

qint64 PlayerNotification::drawRegular(KaraokePainter &p)
{
    m_customFont.setPointSize( p.largetsFontSize( m_customFont, p.rect().width(), m_customMessage ));
    p.setFont( m_customFont );
    p.drawCenteredOutlineText( 50, Qt::white, m_customMessage );
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

void PlayerNotification::setMessage(const QString &message)
{
    QMutexLocker m( &m_mutex );
    m_customMessage = message;
}

void PlayerNotification::clearMessage()
{
    QMutexLocker m( &m_mutex );
    m_customMessage.clear();
}

void PlayerNotification::reset()
{
    m_textOffset = 0;
    m_scrollOffset = 0;
}
