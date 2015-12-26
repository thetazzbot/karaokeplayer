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
