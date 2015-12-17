#ifndef EVENTCONTROLLER_H
#define EVENTCONTROLLER_H

#include <QString>
#include <QKeyEvent>

class EventController_DBus;

class EventController : public QObject
{
    Q_OBJECT

    public:
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

        // EVents from different sources
        void    keyEvent( QKeyEvent * event );
        void    lircEvent( QString event );
        void    dbusEvent( QString event );

    private:
        EventController_DBus *   m_dbusController;
};

extern EventController * pController;

#endif // EVENTCONTROLLER_H
