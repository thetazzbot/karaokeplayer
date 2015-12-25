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
        QList<SongQueue::Song>  songQueue;

    signals:

    public slots:

};

extern CurrentState * pCurrentState;

#endif // CURRENTSTATE_H
