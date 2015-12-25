#include <QPainter>
#include <QThread>

#include "karaokesong.h"
#include "karaokewidget.h"
#include "songqueue.h"
#include "actionhandler.h"
#include "playerrenderer.h"
#include "playernotification.h"


KaraokeWidget::KaraokeWidget( QWidget *parent )
    : QWidget(parent)
{
    m_karaoke = 0;

    // 0 is set as draw image
    m_images[0] = new QImage(100, 100, QImage::Format_ARGB32);
    m_images[0]->fill( Qt::black );
    m_drawImageIndex = 0;

    // 1 as render image
    m_images[1] = new QImage(100, 100, QImage::Format_ARGB32);
    m_renderer = new PlayerRenderer( pNotification, m_images[1], this );
    m_renderer->start();
}

KaraokeWidget::~KaraokeWidget()
{
    delete m_images[0];
    delete m_images[1];
}

void KaraokeWidget::startKaraoke(KaraokeSong *k)
{
    m_karaokeMutex.lock();
    m_karaoke = k;
    m_karaokeMutex.unlock();

    k->start();
}

void KaraokeWidget::stopKaraoke()
{
    m_karaokeMutex.lock();
    KaraokeSong * k = m_karaoke;
    m_karaoke = 0;
    m_karaokeMutex.unlock();

    if ( k )
        k->stop();

    delete k;
}

void KaraokeWidget::stopEverything()
{
    stopKaraoke();

    // Stop the rendering thread
    m_renderer->stop();
    delete m_renderer;
    m_renderer = 0;
}

// This function is called from a different thread, which is guaranteed not to use render image until the call returns
QImage * KaraokeWidget::switchImages()
{
    int newRenderImage;

    // The current draw image, however, can be touched in draw(),
    // so we should not touch it until we switch the pointer
    m_imageMutex.lock();

    if ( m_drawImageIndex == 0 )
    {
        newRenderImage = 0;
        m_drawImageIndex = 1;
    }
    else
    {
        newRenderImage = 1;
        m_drawImageIndex = 0;
    }

    m_imageMutex.unlock();

    // We have switched the draw image which now uses our image; see if we need to resize the current QImage before returning it
    if ( m_images[newRenderImage]->size() != size() )
    {
        delete m_images[newRenderImage];
        m_images[newRenderImage] = new QImage( size(), QImage::Format_ARGB32 );
    }

    // Queue a refresh event for ourselves - note Qt::QueuedConnection as we're calling from a different thread!
    QMetaObject::invokeMethod( this, "update", Qt::QueuedConnection );

    return m_images[newRenderImage];
}

void KaraokeWidget::paintEvent(QPaintEvent *)
{
    QPainter p( this );
    QRect prect = QRect( 0, 0, width(), height() );

    m_imageMutex.lock();
    p.drawImage( prect, *m_images[ m_drawImageIndex ] );
    m_imageMutex.unlock();
}

void KaraokeWidget::keyPressEvent(QKeyEvent *event)
{
    pActionHandler->keyEvent( event );
}
