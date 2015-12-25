#ifndef PLAYERBACKGROUNDVIDEO_H
#define PLAYERBACKGROUNDVIDEO_H

#include <QImage>
#include <QMutex>
#include <QAtomicInt>
#include <QMediaPlayer>
#include <QVideoSurfaceFormat>
#include <QVideoFrame>
#include <QAbstractVideoSurface>

#include "player.h"
#include "playerbackground.h"


class PlayerBackgroundVideo : public QAbstractVideoSurface, public PlayerBackground
{
    Q_OBJECT

    public:
        PlayerBackgroundVideo();
        virtual ~PlayerBackgroundVideo();

        // Background could be initialized either from the settings (by callign initFromSettings() or from
        // a specific file/QIODevice - for example for KFN files. If the background cannot be initialized
        // a specific way, it must return an error.
        virtual bool    initFromSettings( const QString& param = "" );
        virtual bool    initFromFile( QIODevice * file );

        // Draws the background on the image; the prior content of the image is undefined. If false is returned,
        // it is considered an error, and the whole playing process is aborted - use wisely
        virtual qint64  draw( KaraokePainter& p );

        // Current state notifications, may be reimplemented. Default implementation does nothing.
        virtual void    start();
        virtual void    pause( bool pausing );
        virtual void    stop();

        // This one is overridden from QAbstractVideoSurface
        bool start(const QVideoSurfaceFormat & format);

        // This one is overridden from QAbstractVideoSurface
        bool present(const QVideoFrame &frame);

        // This one is overridden from QAbstractVideoSurface
        QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;

    private slots:
        void	slotError(QMediaPlayer::Error error);
        void    slotMediaStatusChanged(QMediaPlayer::MediaStatus status);

    private:
        QMediaPlayer        m_player;
        QImage::Format      m_frameFormat;
        QVideoFrame         m_lastFrame;
        QRect               m_viewport;
        QMutex              m_mutex;
        bool                m_flipped;
        QAtomicInt          m_valid;

        bool                m_startPlaying;
};

#endif // PLAYERBACKGROUNDVIDEO_H
