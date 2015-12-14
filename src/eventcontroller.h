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
        void    playerRewind();

        // Events for queue manager
        void    queueAdd( QString songpath, QString singer );
        void    queueNext();
        void    queuePrevious(); // does restart if not within the first 5 seconds
        void    queueClear();

    public slots:
        // Events from player
        void    playerSongFinished( QString fileName );
        void    playerSongFailed( QString fileName );

        // EVents from different sources
        void    keyEvent( QKeyEvent * event );
        void    lircEvent( QString event );
        void    dbusEvent( QString event );

    private:
        EventController_DBus *   m_dbusController;
};

extern EventController * pController;

#endif // EVENTCONTROLLER_H
