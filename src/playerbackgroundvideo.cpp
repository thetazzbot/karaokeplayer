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

#include <QDebug>

#include "settings.h"
#include "karaokepainter.h"
#include "actionhandler.h"
#include "currentstate.h"
#include "playerbackgroundvideo.h"


PlayerBackgroundVideo::PlayerBackgroundVideo()
    : QAbstractVideoSurface(), PlayerBackground(), m_player( this, QMediaPlayer::VideoSurface )
{
    m_frameFormat = QImage::Format_Invalid;
    m_valid = 0;
    m_startPlaying = false;

    m_player.setVideoOutput( this );
    m_player.setMuted( true );

    connect( &m_player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(slotError(QMediaPlayer::Error)) );
    connect( &m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)) );
}

PlayerBackgroundVideo::~PlayerBackgroundVideo()
{
}

bool PlayerBackgroundVideo::initFromFile(QIODevice *)
{
    return false;
}

void PlayerBackgroundVideo::start()
{
    if ( m_startPlaying )
        m_player.play();
    else
        m_startPlaying = true;
}

void PlayerBackgroundVideo::pause(bool pausing)
{
    if ( pausing )
        m_player.pause();
    else
        m_player.play();
}

void PlayerBackgroundVideo::stop()
{
    // Store the current video position so next song could resume video
    if ( m_player.position() > 0 )
        pSettings->m_playerBackgroundLastVideoTime = m_player.position();

    m_player.stop();
}

bool PlayerBackgroundVideo::start(const QVideoSurfaceFormat &format)
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

bool PlayerBackgroundVideo::present(const QVideoFrame &frame)
{
    QMutexLocker m( &m_mutex );
    m_lastFrame = frame;

    return true;
}

QList<QVideoFrame::PixelFormat> PlayerBackgroundVideo::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
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

void PlayerBackgroundVideo::slotError(QMediaPlayer::Error error)
{
    qDebug() << "video" << error;
}

void PlayerBackgroundVideo::slotMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << "video" << status;

    if ( status == QMediaPlayer::LoadedMedia )
    {
        if ( m_startPlaying )
            m_player.play();
        else
            m_startPlaying = true;
    }

    if ( status == QMediaPlayer::EndOfMedia )
    {
        pSettings->m_playerBackgroundLastObject++;

        if ( (int) pSettings->m_playerBackgroundLastObject >= pSettings->m_playerBackgroundObjects.size() )
            pSettings->m_playerBackgroundLastObject = 0;

        pSettings->m_playerBackgroundLastVideoTime = 0;

        QString videofile = pSettings->m_playerBackgroundObjects[ pSettings->m_playerBackgroundLastObject ];
        qDebug("Background video ended, switching to video %s", qPrintable(videofile) );

        m_player.setMedia( QMediaContent( QUrl::fromLocalFile( videofile ) ) );
        m_player.setVideoOutput( this );
        m_player.setMuted( true );
    }
}

bool PlayerBackgroundVideo::initFromSettings(const QString &)
{
    m_valid = 0;

    QString videofile = pSettings->m_playerBackgroundObjects[ pSettings->m_playerBackgroundLastObject ];

    m_player.setMedia( QMediaContent( QUrl::fromLocalFile( videofile ) ) );
    m_player.setPosition( pSettings->m_playerBackgroundLastVideoTime );

    return true;
}

qint64 PlayerBackgroundVideo::draw(KaraokePainter &p)
{
    if ( m_valid )
    {
        QMutexLocker m( &m_mutex );

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

    // Next frame
    return 0;
}
