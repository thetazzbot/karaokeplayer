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

#ifndef ACTIONHANDLER_H
#define ACTIONHANDLER_H

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

            ACTION_PLAYER_RATING_DECREASE,
            ACTION_PLAYER_RATING_INCREASE,

            ACTION_QUIT
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
        void    playerRatingIncrease();
        void    playerRatingDecrease();

        // Actions for queue manager
        void    queueAdd( QString singer, int id );
        void    queueAdd( QString singer, QString path );
        void    queueNext();
        void    queuePrevious(); // does restart if not within the first 5 seconds
        void    queueClear();

        // Generic actions
        void    actionQuit();

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

#endif // ACTIONHANDLER_H
