#ifndef EVENTCONTROLLER_H
#define EVENTCONTROLLER_H

#include <QString>
#include <QKeyEvent>

#define HAS_DBUS_SUPPORT
#define HAS_LIRC_SUPPORT
#define HAS_HTTP_SUPPORT

class ActionHandler_WebServer;


//
// ActionHandler receives events (from keyboard, remote, dbus, webserver...) and translated them into actions which it emits as signals.
// For example, when you press STOP button on remote, the LIRC action handler gets it, translates into Action and posts it via cmdAction(ACTION_PLAYER_STOP)
// and the handler emits playerStop() to which everyone who needs it could connect.
//
class ActionHandler : public QObject
{
    Q_OBJECT

    public:
        enum Action
        {
            ACTION_PLAYER_START,
            ACTION_PLAYER_PAUSERESUME,
            ACTION_PLAYER_STOP,
            ACTION_PLAYER_SEEK_BACK,
            ACTION_PLAYER_SEEK_FORWARD,
            ACTION_PLAYER_VOLUME_UP,
            ACTION_PLAYER_VOLUME_DOWN,

            ACTION_QUEUE_NEXT,
            ACTION_QUEUE_PREVIOUS,
            ACTION_QUEUE_CLEAR,

            ACTION_LYRIC_EARLIER,   // decreases the music-lyric delay (negative too)
            ACTION_LYRIC_LATER,     // increases the music-lyric delay
        };

        ActionHandler();
        ~ActionHandler();

        // Post-create initialization/shutdown
        void    start();
        void    stop();

        // Looks up the event (for translations)
        static int actionByName( const char * eventname );

    signals:
        // Actions for player
        void    playerStart();
        void    playerStop();
        void    playerPauseResume();
        void    playerForward();
        void    playerBackward();
        void    playerLyricsEarlier();
        void    playerLyricsLater();
        void    playerVolumeUp();
        void    playerVolumeDown();

        // Actions for queue manager
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
        bool    cmdAction( int event );

        // EVents from different sources
        void    keyEvent( QKeyEvent * event );

    private:
        ActionHandler_WebServer   *   m_webserver;

        static QMap< QString, ActionHandler::Action > m_actionNameMap;
};

extern ActionHandler * pActionHandler;

#endif // EVENTCONTROLLER_H
