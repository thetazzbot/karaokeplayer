#ifndef EVENTCONTROLLER_H
#define EVENTCONTROLLER_H

#include <QString>
#include <QKeyEvent>

#define HAS_DBUS_CONTROLLER

class EventController : public QObject
{
    Q_OBJECT

    public:
        enum Event
        {
            EVENT_PLAYER_START,
            EVENT_PLAYER_PAUSERESUME,
            EVENT_PLAYER_STOP,
            EVENT_PLAYER_BACKWARD,
            EVENT_PLAYER_FORWARD,

            EVENT_QUEUE_NEXT,
            EVENT_QUEUE_PREVIOUS,
            EVENT_QUEUE_CLEAR,
        };

        EventController();
        ~EventController();

    signals:
        // Events for player
        void    playerStart();
        void    playerStop();
        void    playerPauseResume();
        void    playerForward();
        void    playerBackward();

        // Events for queue manager
        void    queueAdd( QString songpath, QString singer );
        void    queueNext();
        void    queuePrevious(); // does restart if not within the first 5 seconds
        void    queueClear();

    public slots:
        // Events from player
        void    playerSongFinished();
        void    playerSongFailed();

        // Reports an error
        void    error( QString message );

        // Most events should end up here
        bool    cmdEvent( Event event );

        // EVents from different sources
        void    keyEvent( QKeyEvent * event );
        void    lircEvent( QString event );
        void    dbusEvent( QString event );

    private:

};

extern EventController * pController;

#endif // EVENTCONTROLLER_H
