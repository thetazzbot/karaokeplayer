#include <QPainter>
#include <QThread>

#include "playerwidget.h"


PlayerWidget::PlayerWidget( QWidget *parent )
    : QWidget(parent), m_image( 800, 600, QImage::Format_ARGB32 ), m_mutex( QMutex::Recursive )
{
    m_image.fill( Qt::black );
}

QImage &PlayerWidget::acquireImage()
{
    m_mutex.lock();
    return m_image;
}

void PlayerWidget::releaseImage() const
{
    m_mutex.unlock();
}

void PlayerWidget::refresh()
{
    update();
}

/*
void PlayerWidget::updateLyrics(qint64 time)
{
    if ( m_nextRedrawTime != -1 && time < m_nextRedrawTime )
        return;

    m_lastRedrawTime = time;
    redrawLyrics();
}

void PlayerWidget::redrawLyrics()
{
    KaraokePainter p( KaraokePainter::AREA_MAIN_SCREEN, m_lastRedrawTime, m_image );

    if ( m_customText.isEmpty() )
    {
        m_backgroundRenderer->draw( p );
        m_lyricRenderer->draw( p );

        m_nextRedrawTime = m_lyricRenderer->nextUpdate();
    }
    else
    {
        p.drawCenteredOutlineText( 50, Qt::white, m_customText );
    }

    update();
}
*/
void PlayerWidget::paintEvent(QPaintEvent *)
{
    QPainter p( this );
    QRect prect = QRect( 0, 0, width() - 1, height() - 1 );

    p.fillRect( prect, Qt::black );

    m_mutex.lock();
 //  QThread::sleep(1);
    p.drawImage( prect, m_image );
    m_mutex.unlock();
}
/*
void PlayerWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent( event );

    m_mutex.lock();

    if ( m_image.size() != size() )
        m_image = m_image.scaled( size() );

    m_mutex.unlock();
}
*/
