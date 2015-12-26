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

#ifndef CURRENTSTATE_H
#define CURRENTSTATE_H

#include <QObject>
#include <QAtomicInt>
#include "songqueue.h"

//
// Represents current state of a player.
//
// This is a storage of relatively long-living state (which song is playing, what is current player position, is Web server running and on which address, etc)
// which are supplied by relevant modules (queue, player), and are available to all modules.
//
//
class CurrentState : public QObject
{
    Q_OBJECT

    public:
        enum
        {
            // Check Player widget when modifying this enum (it checks for ==PLAYERSTATE_PLAYING)!
            PLAYERSTATE_STOPPED,
            PLAYERSTATE_PLAYING,
            PLAYERSTATE_PAUSED
        };

        explicit CurrentState(QObject *parent = 0);

        // mseconds per frame
        unsigned int            msecPerFrame;

        // Web server URL
        QString                 webserverURL;

        // Current player state
        QAtomicInt              playerState;

        // Current song being played (not valid if playerState is STOPPED)
        SongQueue::Song         playerSong;

        // Current song parameters (not valid if playerState is STOPPED) - updated from another thread!
        QAtomicInt              playerPosition;
        QAtomicInt              playerDuration;

        int                     playerVolume;

        // Current singer queue (read-only; any modifications will be overwritten)
        //QList<SongQueue::Song>  songQueue;

        // If true, this is a single-window mode
        bool                    modeFullscreen;

    signals:

    public slots:

};

extern CurrentState * pCurrentState;

#endif // CURRENTSTATE_H
