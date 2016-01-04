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

#include "logger.h"
#include "settings.h"
#include "karaokepainter.h"
#include "actionhandler.h"
#include "currentstate.h"
#include "backgroundvideo.h"


BackgroundVideo::BackgroundVideo()
    : QObject(), Background()
{
    connect( &m_player, SIGNAL(finished()), this, SLOT(finished()) );
}

BackgroundVideo::~BackgroundVideo()
{
}

void BackgroundVideo::start()
{
    m_player.play();
}

void BackgroundVideo::pause(bool pausing)
{
    m_player.pauseOrResume( pausing );
}

void BackgroundVideo::stop()
{
    // Store the current video position so next song could resume video
    if ( m_player.position() > 0 )
        pCurrentState->playerBackgroundLastVideoPosition = m_player.position();

    m_player.stop();
}


void BackgroundVideo::finished()
{
    Logger::debug("Background video: current video ended, switching to new video" );
    pCurrentState->nextBackgroundObject();

    initFromSettings();
}

bool BackgroundVideo::initFromSettings()
{
    QString videofile = pCurrentState->currentBackgroundObject();

    if ( videofile.isEmpty() || !m_player.loadVideo( videofile, true ) )
    {
        Logger::debug("Background video: video file %s failed to load", qPrintable(videofile) );
        return false;
    }

    Logger::debug("Background video: video file %s loaded successfully", qPrintable(videofile) );

    m_player.seekTo( pCurrentState->playerBackgroundLastVideoPosition );
    return true;
}

bool BackgroundVideo::initFromFile(QIODevice *, const QString &)
{
    // FIXME: This is not unused (i.e. UltraStart and KFN use it), but not currently implemented
    return false;
}

qint64 BackgroundVideo::draw(KaraokePainter &p)
{
    m_player.draw( p );

    // Next frame
    return 0;
}
