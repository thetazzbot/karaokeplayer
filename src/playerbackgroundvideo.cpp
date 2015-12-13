#include <QVideoSurfaceFormat>

#include "karaokepainter.h"
#include "playerbackgroundvideo.h"

PlayerBackgroundVideo::PlayerBackgroundVideo()
    : PlayerBackground()
{
    m_frameFormat = QImage::Format_Invalid;
}

PlayerBackgroundVideo::~PlayerBackgroundVideo()
{

}

bool PlayerBackgroundVideo::initFromFile(QIODevice &file)
{
    return false;
}

void PlayerBackgroundVideo::start()
{
    m_player.play();
}

void PlayerBackgroundVideo::pause(bool resuming)
{
    if ( resuming )
        m_player.play();
    else
        m_player.pause();
}

bool PlayerBackgroundVideo::initFromSettings(const QString &param)
{
    m_player.setMedia( QMediaContent( QUrl::fromLocalFile("/home/tim/work/my/karaokeplayer/test/bg.avi")) );
    m_player.setMuted( true );
    m_player.setVideoOutput( this );

    return true;
}

qint64 PlayerBackgroundVideo::draw(KaraokePainter &p)
{
    /*
    m_mutex.lock();
    if ( m_currentFrame.map(QAbstractVideoBuffer::ReadOnly) )
    {
        const QTransform oldTransform = p.transform();

        if ( surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop )
        {
           p.scale(1, -1);
           p.translate(0, -p.size().height());
        }

        p.drawImage( p.rect(), image, surfaceFormat().viewport() );
        p.setTransform(oldTransform);

        m_currentFrame.unmap();
    }
    m_mutex.unlock();
    */


    m_mutex.lock();
    p.drawImage( p.rect(), m_lastFrame, surfaceFormat().viewport() );
    m_mutex.unlock();

    // Assume 25fps
    return p.time() + 40;
}

bool PlayerBackgroundVideo::present(const QVideoFrame &frame)
{
    QVideoFrame f = frame;
    if ( f.map(QAbstractVideoBuffer::ReadOnly) )
    {
/*        const QTransform oldTransform = p.transform();

        if ( surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop )
        {
           p.scale(1, -1);
           p.translate(0, -p.size().height());
        }
*/
        if ( m_frameFormat == QImage::Format_Invalid )
            m_frameFormat = QVideoFrame::imageFormatFromPixelFormat( surfaceFormat().pixelFormat() );

        m_mutex.lock();
        m_lastFrame = QImage( f.bits(), f.width(), f.height(), f.bytesPerLine(), m_frameFormat );
        m_mutex.unlock();

//        p.drawImage( p.rect(), image, surfaceFormat().viewport() );
//        p.setTransform(oldTransform);

        f.unmap();
    }

    return true;
}

QList<QVideoFrame::PixelFormat> PlayerBackgroundVideo::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    if (handleType == QAbstractVideoBuffer::NoHandle) {
        return QList<QVideoFrame::PixelFormat>()
                << QVideoFrame::Format_RGB32
                << QVideoFrame::Format_ARGB32
                << QVideoFrame::Format_ARGB32_Premultiplied
                << QVideoFrame::Format_RGB565
                << QVideoFrame::Format_RGB555;
    } else {
        return QList<QVideoFrame::PixelFormat>();
    }
}

