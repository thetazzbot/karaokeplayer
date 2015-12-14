#include <QPainter>
#include <QThread>

#include "eventcontroller.h"
#include "playerwidget.h"


PlayerWidget::PlayerWidget( QWidget *parent )
    : QWidget(parent), m_image( 800, 600, QImage::Format_ARGB32 ), m_mutex( QMutex::Recursive )
{
    m_image.fill( Qt::black );
}

bool PlayerWidget::setImage( QImage &img )
{
    if ( size() != img.size() )
    {
        img = QImage( size(), QImage::Format_ARGB32 );
        return false;
    }

    m_mutex.lock();
    m_image = img;
    m_mutex.unlock();

    return true;
}

void PlayerWidget::refresh()
{
    update();
}

void PlayerWidget::paintEvent(QPaintEvent *)
{
    QPainter p( this );
    QRect prect = QRect( 0, 0, width() - 1, height() - 1 );

    p.fillRect( prect, Qt::black );

    m_mutex.lock();
    p.drawImage( prect, m_image );
    m_mutex.unlock();
}

void PlayerWidget::keyPressEvent(QKeyEvent *event)
{
    pController->keyEvent( event );
}
