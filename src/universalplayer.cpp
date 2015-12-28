/**************************************************************************
 *  Ulduzsoft Karaoke PLayer - cross-platform desktop karaoke player      *
 *  Copyright (C) 2015-2016 George Yunaev, support@ulduzsoft.com          *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *																	      *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#include <QMutex>
#include <QImage>
#include <QThread>
#include <QVideoSurfaceFormat>
#include <QVideoFrame>
#include <QAbstractVideoSurface>
#include <QElapsedTimer>
#include <QApplication>
#include <QFile>

#include "logger.h"
#include "actionhandler.h"
#include "currentstate.h"
#include "karaokepainter.h"
#include "universalplayer.h"


// Background video surface implementation
class UniversalPlayerVideoSurface : public QAbstractVideoSurface
{
    public:
        UniversalPlayerVideoSurface()
        {
            m_frameFormat = QImage::Format_Invalid;
            m_valid = 0;
            m_flipped = false;
        }

    // Draws the frame on the image
    void draw( KaraokePainter& p )
    {
        QMutexLocker m( &m_mutex );

        if ( m_valid )
        {
            if ( m_lastFrame.map(QAbstractVideoBuffer::ReadOnly) )
            {
                // This is an unlikely case, so we'd rather have two code blocks to exclude unnecessary operations
                if ( m_flipped )
                {
                    const QTransform oldTransform = p.transform();
                    p.scale(1, -1);
                    p.translate(0, -p.size().height());
                    p.drawImage( p.rect(), QImage( m_lastFrame.bits(), m_lastFrame.width(), m_lastFrame.height(), m_lastFrame.bytesPerLine(), m_frameFormat ), m_viewport );
                    p.setTransform(oldTransform);
                }
                else
                {
                    p.drawImage( p.rect(), QImage( m_lastFrame.bits(), m_lastFrame.width(), m_lastFrame.height(), m_lastFrame.bytesPerLine(), m_frameFormat ), m_viewport );
                }
            }

            m_lastFrame.unmap();
        }
    }

    // Overridden from QAbstractVideoSurface
    virtual bool start( const QVideoSurfaceFormat &format )
    {
        m_mutex.lock();

        m_flipped = ( format.scanLineDirection() == QVideoSurfaceFormat::BottomToTop );
        m_frameFormat = QVideoFrame::imageFormatFromPixelFormat( format.pixelFormat() );
        m_viewport = format.viewport();
        m_valid = 1;

        // We want to unlock it before calling the parent function, which may call present() and deadlock
        m_mutex.unlock();

        return QAbstractVideoSurface::start( format );
    }

    virtual void stop()
    {
        m_mutex.lock();
        m_valid = 0;
        m_mutex.unlock();

        QAbstractVideoSurface::stop();
    }

    // This one is overridden from QAbstractVideoSurface
    virtual bool present( const QVideoFrame &frame )
    {
        QMutexLocker m( &m_mutex );
        m_lastFrame = frame;

        return true;
    }

    // This one is overridden from QAbstractVideoSurface
    virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
    {
        if ( handleType == QAbstractVideoBuffer::NoHandle )
        {
            return QList<QVideoFrame::PixelFormat>()
                    << QVideoFrame::Format_RGB32
                    << QVideoFrame::Format_ARGB32
                    << QVideoFrame::Format_ARGB32_Premultiplied
                    << QVideoFrame::Format_RGB565
                    << QVideoFrame::Format_RGB555;
        }
        else
        {
            return QList<QVideoFrame::PixelFormat>();
        }
    }

    bool isValid() const
    {
        QMutexLocker m( &m_mutex );
        return m_valid;
    }

    private:
        QImage::Format      m_frameFormat;
        QVideoFrame         m_lastFrame;
        QRect               m_viewport;
        mutable QMutex      m_mutex;
        bool                m_flipped;
        bool                m_valid;
};



UniversalPlayer::UniversalPlayer( QObject *parent )
    : QObject(parent)
{
    m_surface = 0;
    m_drawEnabled = 0;
    m_player = 0;
}

UniversalPlayer::~UniversalPlayer()
{
    if ( m_player )
    {
        m_player->stop();
        m_player->deleteLater();
    }
}

bool UniversalPlayer::loadAudio( const QString &musicfile )
{
    newMediaPlayer( false, false );

    m_mediaLoadedOrFailed = 0;
    m_player->setMedia( QMediaContent( QUrl::fromLocalFile( musicfile ) ) );

    return waitForMediaLoaded();
}

bool UniversalPlayer::loadVideo(const QString &videofile , bool muted)
{
    newMediaPlayer( true, muted );

    // Prepare the video output surface if we do not have it yet
    if ( !m_surface )
        m_surface = new UniversalPlayerVideoSurface();

    m_player->setVideoOutput( m_surface );

    m_mediaLoadedOrFailed = 0;
    m_player->setMedia( QMediaContent( QUrl::fromLocalFile( videofile ) ) );

    return waitForMediaLoaded();
}

void UniversalPlayer::draw(KaraokePainter &p)
{
    if ( m_drawEnabled )
    {
        QMutexLocker m( &m_drawMutex );

        // This value may change after the mutex is locked!
        if ( m_drawEnabled )
            m_surface->draw( p );
    }
}

void UniversalPlayer::play()
{
    m_player->play();

    if ( m_surface )
        m_drawEnabled = 1;
}

void UniversalPlayer::stop()
{
    m_drawMutex.lock();
    m_drawEnabled = 0;
    m_drawMutex.unlock();

    if ( m_player )
        m_player->stop();
}

void UniversalPlayer::pauseOrResume(bool pause)
{
    if ( pause )
        m_player->play();
    else
        m_player->pause();
}

void UniversalPlayer::seekTo(qint64 time)
{
    m_player->setPosition( time );
}

qint64 UniversalPlayer::position()
{
    return m_player->position();
}

void UniversalPlayer::volumeDown()
{
    int newvolume = qMax( m_player->volume() - 10, 0 );
    m_player->setVolume( newvolume );
    pCurrentState->playerVolume = newvolume;
}

void UniversalPlayer::volumeUp()
{
    int newvolume = qMin( m_player->volume() + 10, 100 );
    m_player->setVolume( newvolume );
    pCurrentState->playerVolume = newvolume;
}

qint64 UniversalPlayer::duration()
{
    return m_player->duration();
}

void UniversalPlayer::slotError(QMediaPlayer::Error )
{
    m_errorMsg = m_player->errorString();
    m_mediaLoadedOrFailed = 1;
}

void UniversalPlayer::slotMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if ( status == QMediaPlayer::EndOfMedia )
        emit finished();

    if ( status == QMediaPlayer::LoadedMedia )
    {
        pCurrentState->playerDuration = m_player->duration();
        pCurrentState->playerVolume = m_player->volume();

        m_errorMsg.clear();
        m_mediaLoadedOrFailed = 1;
    }
}

bool UniversalPlayer::waitForMediaLoaded()
{
    Logger::debug("UniversalPlayer: waiting for media loaded" );

    while ( m_mediaLoadedOrFailed == 0 )
        QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );

    Logger::debug("UniversalPlayer: media loaded" );
    return m_errorMsg.isEmpty();
}

void UniversalPlayer::newMediaPlayer( bool video, bool muted )
{
    if ( m_player )
        delete m_player;

    // Use Qt media player
    if ( video )
        m_player = new QMediaPlayer( 0, QMediaPlayer::VideoSurface );
    else
        m_player = new QMediaPlayer( 0 );

    connect( m_player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(slotError(QMediaPlayer::Error)) );
    connect( m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)) );

    if ( !muted )
    {
        connect( pActionHandler, SIGNAL(playerVolumeDown()), this, SLOT(volumeDown()) );
        connect( pActionHandler, SIGNAL(playerVolumeUp()), this, SLOT(volumeUp()) );
    }
    else
        m_player->setMuted( true );
}
