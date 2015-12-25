#ifndef EVENTCONTROLLER_H
#define EVENTCONTROLLER_H

#include <QString>
#include <QKeyEvent>

#define HAS_DBUS_SUPPORT
#define HAS_LIRC_SUPPORT
#define HAS_HTTP_SUPPORT

class EventController_WebServer;


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

            EVENT_LYRIC_EARLIER,   // decreases the music-lyric delay (negative too)
            EVENT_LYRIC_LATER,     // increases the music-lyric delay
        };

        EventController();
        ~EventController();

        // Post-create initialization/shutdown
        void    start();
        void    stop();

        // Looks up the event (for translations)
        static int eventByName( const char * eventname );

    signals:
        // Events for player
        void    playerStart();
        void    playerStop();
        void    playerPauseResume();
        void    playerForward();
        void    playerBackward();
        void    playerLyricsEarlier();
        void    playerLyricsLater();

        // Events for queue manager
        void    queueAdd( QString songpath, QString singer );
        void    queueNext();
        void    queuePrevious(); // does restart if not within the first 5 seconds
        void    queueClear();

    public slots:
        // Events from player
        void    playerSongFinished();
        void    playerSongFailed();

        // Reports an error or warning
        void    error( QString message );
        void    warning( QString message );

        // Most events should end up here
        bool    cmdEvent( int event );

        // EVents from different sources
        void    keyEvent( QKeyEvent * event );

    private:
        EventController_WebServer   *   m_webserver;
};

extern EventController * pController;

#endif // EVENTCONTROLLER_H
