#ifndef CURRENTSTATE_H
#define CURRENTSTATE_H

#include <QObject>

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
        explicit CurrentState(QObject *parent = 0);

        QString     webserverURL;

    signals:

    public slots:

};

extern CurrentState * pCurrentState;

#endif // CURRENTSTATE_H
