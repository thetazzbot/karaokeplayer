#include <QPainter>

#include "playerlyrics.h"
#include "playerbackground.h"
#include "playerwidget.h"

PlayerWidget::PlayerWidget( PlayerLyrics * lyricRenderer, PlayerBackground  * bgRenderer, QWidget *parent )
    : QWidget(parent), m_image( 800, 600, QImage::Format_ARGB32 )
{
    m_lyricRenderer = lyricRenderer;
    m_backgroundRenderer = bgRenderer;
    m_nextRedrawTime = -1;
}

void PlayerWidget::updateLyrics(qint64 time)
{
    if ( m_nextRedrawTime != -1 && time < m_nextRedrawTime )
        return;

    m_lastRedrawTime = time;
    redrawLyrics();
}

void PlayerWidget::redrawLyrics()
{
    m_backgroundRenderer->draw( m_lastRedrawTime, m_image );
    m_lyricRenderer->draw( m_lastRedrawTime, m_image );

    m_nextRedrawTime = m_lyricRenderer->nextUpdate();
    update();
}

void PlayerWidget::paintEvent(QPaintEvent *)
{
    QPainter p( this );
    QRect prect = QRect( 0, 0, width() - 1, height() - 1 );

    p.fillRect( prect, Qt::black );

    p.drawImage( prect, m_image );
}

void PlayerWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent( event );

    if ( m_image.size() != size() )
    {
        m_image = QImage( size(), QImage::Format_ARGB32 );
        redrawLyrics();
    }
}
