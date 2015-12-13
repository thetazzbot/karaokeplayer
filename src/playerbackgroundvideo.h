#ifndef PLAYERBACKGROUNDVIDEO_H
#define PLAYERBACKGROUNDVIDEO_H

#include <QMutex>
#include <QImage>
#include <QMediaPlayer>
#include <QAbstractVideoSurface>

#include "playerbackground.h"


class PlayerBackgroundVideo : public PlayerBackground, public QAbstractVideoSurface
{
    public:
        PlayerBackgroundVideo();
        virtual ~PlayerBackgroundVideo();

        // Background could be initialized either from the settings (by callign initFromSettings() or from
        // a specific file/QIODevice - for example for KFN files. If the background cannot be initialized
        // a specific way, it must return an error.
        virtual bool    initFromSettings( const QString& param = "" );
        virtual bool    initFromFile( QIODevice& file );

        // Draws the background on the image; the prior content of the image is undefined. If false is returned,
        // it is considered an error, and the whole playing process is aborted - use wisely
        virtual qint64  draw( KaraokePainter& p );

        // Current state notifications, may be reimplemented. Default implementation does nothing.
        virtual void    start();
        virtual void    pause( bool resuming );


        //
        // QAbstractVideoSurface
        //
        QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
        bool present( const QVideoFrame &frame );

    private:
        QMediaPlayer        m_player;

        QImage::Format      m_frameFormat;

        QImage              m_lastFrame;
        QMutex              m_mutex;

};

#endif // PLAYERBACKGROUNDVIDEO_H
