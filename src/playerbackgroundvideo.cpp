#include <QVideoSurfaceFormat>

#include "karaokepainter.h"
#include "playerbackgroundvideo.h"

PlayerBackgroundVideo::PlayerBackgroundVideo()
    : PlayerBackground()
{
}

PlayerBackgroundVideo::~PlayerBackgroundVideo()
{

}

bool PlayerBackgroundVideo::initFromFile(QIODevice &file)
{

}

bool PlayerBackgroundVideo::initFromSettings(const QString &param)
{
    m_player.setMedia( QMediaContent( QUrl::fromLocalFile("/home/tim/work/my/karaokeplayer/test/bg.avi")) );
    m_player.setMuted( true );
    m_player.setVideoOutput( this );
    m_player.play();
}

bool PlayerBackgroundVideo::needUpdate() const
{
    return true;
}

bool PlayerBackgroundVideo::draw(KaraokePainter &p)
{
    if ( m_currentFrame.map(QAbstractVideoBuffer::ReadOnly) )
    {
        const QTransform oldTransform = p.transform();

        if ( surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop )
        {
           p.scale(1, -1);
           p.translate(0, -p.size().height());
        }

        QImage image(
                m_currentFrame.bits(),
                m_currentFrame.width(),
                m_currentFrame.height(),
                m_currentFrame.bytesPerLine(),
                QVideoFrame::imageFormatFromPixelFormat( surfaceFormat().pixelFormat() ) );

        p.drawImage( p.rect(), image, surfaceFormat().viewport() );
        p.setTransform(oldTransform);

        m_currentFrame.unmap();
    }
}

bool PlayerBackgroundVideo::present(const QVideoFrame &frame)
{
    m_currentFrame = frame;
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

