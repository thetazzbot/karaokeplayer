#include <QTime>

#include "karaokefile.h"
#include "playernotification.h"
#include "playerrenderer.h"
#include "playerwidget.h"
#include "settings.h"

static const unsigned int RENDER_FPS = 20;


PlayerRenderer::PlayerRenderer(PlayerNotification *notification, QImage * render, PlayerWidget *parent)
    : QThread(parent)
{
    m_widget = parent;
    m_notification = notification;
    m_continue = 1;
    m_renderImage = render;
}

PlayerRenderer::~PlayerRenderer()
{
    if ( m_continue )
    {
        m_continue.store( 0 );
        wait();
    }
}

void PlayerRenderer::run()
{
    // from http://www.koonsolo.com/news/dewitters-gameloop/
    // We do not need fast rendering, and need constant FPS, so 2nd solution works well
    QTime next_cycle;
    qint64 remainingms = -1;

    while ( m_continue )
    {
        next_cycle = QTime::currentTime().addMSecs( pSettings->m_playerRenderMSecPerFrame );

        // Prepare the image
        m_renderImage->fill( Qt::black );

        // Use this painter
        KaraokePainter p( m_renderImage );

        // Render the top area notification
        m_notification->drawTop( p, remainingms );

        // Switch the painter to main area
        p.setClipAreaMain();

        // If we have the Karaoke object, use it
        m_widget->m_karaokeMutex.lock();

        if ( m_widget->m_karaoke )
        {
            // Set timing
            // FIXME
            p.setTimes( m_widget->m_karaoke->position(), m_widget->m_karaoke->duration() );
            remainingms = m_widget->m_karaoke->duration() - m_widget->m_karaoke->position();

            // Draw background and lyrics
            m_widget->m_karaoke->draw( p );
        }
        else
            remainingms = -1;

        m_widget->m_karaokeMutex.unlock();

        // And subsequent notifications, if any (they must go on top of lyrics)
        p.setClipAreaText();
        m_notification->drawRegular( p );
        p.end();

        // And finally render it
        m_renderImage = m_widget->switchImages();

        // When should we have next tick?
        int remainingms = qMax( QTime::currentTime().msecsTo( next_cycle ), 5 );
        msleep( remainingms );
    }
}
