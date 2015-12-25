#include <QDBusConnection>
#include <QApplication>
#include <QInputDialog>

#include "actionhandler.h"
#include "actionhandler_dbus.h"
#include "songenqueuedialog.h"
#include "songsearchdialog.h"
#include "songdatabase.h"
#include "settings.h"

#if defined (HAS_LIRC_SUPPORT)
    #include "actionhandler_lirc.h"
#endif

#if defined (HAS_HTTP_SUPPORT)
    #include "actionhandler_webserver.h"
#endif


ActionHandler * pActionHandler;
QMap< QString, ActionHandler::Action > ActionHandler::m_actionNameMap;

ActionHandler::ActionHandler()
    : QObject()
{
    m_webserver = 0;

    // Form the map
    m_actionNameMap["START"] = ACTION_PLAYER_START;
    m_actionNameMap["PAUSE"] = ACTION_PLAYER_PAUSERESUME;
    m_actionNameMap["STOP"] = ACTION_PLAYER_STOP;
    m_actionNameMap["SEEK_BACK"] = ACTION_PLAYER_SEEK_BACK;
    m_actionNameMap["SEEK_FORWARD"] = ACTION_PLAYER_SEEK_FORWARD;
    m_actionNameMap["QUEUE_NEXT"] = ACTION_QUEUE_NEXT;
    m_actionNameMap["QUEUE_PREVIOUS"] = ACTION_QUEUE_PREVIOUS;
    m_actionNameMap["QUEUE_CLEAR"] = ACTION_QUEUE_CLEAR;
    m_actionNameMap["LYRICS_EARLIER"] = ACTION_LYRIC_EARLIER;
    m_actionNameMap["LYRICS_LATER"] = ACTION_LYRIC_LATER;
    m_actionNameMap["VOLUME_UP"] = ACTION_PLAYER_VOLUME_UP;
    m_actionNameMap["VOLUME_DOWN"] = ACTION_PLAYER_VOLUME_DOWN;
}

ActionHandler::~ActionHandler()
{
}

void ActionHandler::start()
{
#if defined (HAS_DBUS_SUPPORT)
    new ActionHandler_DBus( this );

    if ( !QDBusConnection::sessionBus().registerObject( "/", this ) || !QDBusConnection::sessionBus().registerService( DBUS_SERVICE_NAME ) )
        qWarning("Cannot register dbus object");
#endif

#if defined (HAS_LIRC_SUPPORT)
    if ( !pSettings->lircDevicePath.isEmpty()  )
    {
        ActionHandler_LIRC * lirc = new ActionHandler_LIRC( this );

        connect( lirc, SIGNAL(lircEvent(int)), this, SLOT(cmdAction(int)) );
    }
#endif

#if defined (HAS_HTTP_SUPPORT)
    if ( pSettings->httpListenPort > 0 )
    {
        m_webserver = new ActionHandler_WebServer( this );
        m_webserver->start();
    }
#endif
}

void ActionHandler::stop()
{
#if defined (HAS_HTTP_SUPPORT)
    if ( m_webserver )
        m_webserver->exit( 0 );
#endif
}

int ActionHandler::actionByName(const char *eventname)
{
    if ( !m_actionNameMap.contains( eventname ) )
        return -1;

    return (int) m_actionNameMap[eventname];
}

void ActionHandler::playerSongFinished()
{
    emit queueNext();
}

void ActionHandler::playerSongFailed()
{
    emit queueNext();
}

void ActionHandler::error(QString message)
{
    qDebug("ERROR: %s", qPrintable(message));
}

void ActionHandler::warning(QString message)
{
    qDebug("WARNING: %s", qPrintable(message));
}

bool ActionHandler::cmdAction(int event )
{
    switch ( event )
    {
    case ACTION_PLAYER_START:
        emit playerStart();
        break;

    case ACTION_PLAYER_PAUSERESUME:
        emit playerPauseResume();
        break;

    case ACTION_PLAYER_STOP:
        emit playerStop();
        break;

    case ACTION_PLAYER_SEEK_BACK:
        emit playerBackward();
        break;

    case ACTION_PLAYER_SEEK_FORWARD:
        emit playerForward();
        break;

    case ACTION_QUEUE_NEXT:
        emit queueNext();
        break;

    case ACTION_QUEUE_PREVIOUS:
        emit queuePrevious();
        break;

    case ACTION_QUEUE_CLEAR:
        emit queueClear();
        break;

    case ACTION_LYRIC_EARLIER:
        emit playerLyricsEarlier();
        break;

    case ACTION_LYRIC_LATER:
        emit playerLyricsLater();
        break;

    case ACTION_PLAYER_VOLUME_DOWN:
        emit playerVolumeDown();
        break;

    case ACTION_PLAYER_VOLUME_UP:
        emit playerVolumeUp();
        break;

    default:
        return false;
    }

    return true;
}

void ActionHandler::keyEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_Space )
        cmdAction( ACTION_PLAYER_PAUSERESUME );

    if ( event->key() == Qt::Key_Escape )
        cmdAction( ACTION_PLAYER_STOP );

    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
        cmdAction( ACTION_PLAYER_START );

    if ( event->key() == Qt::Key_Left )
        cmdAction( ACTION_PLAYER_SEEK_BACK );

    if ( event->key() == Qt::Key_Right )
        cmdAction( ACTION_PLAYER_SEEK_FORWARD );

    if ( event->key() == Qt::Key_Period )
        cmdAction( ACTION_LYRIC_LATER );

    if ( event->key() == Qt::Key_Comma )
        cmdAction( ACTION_LYRIC_EARLIER );

    if ( event->key() == Qt::Key_Equal )
        cmdAction( ACTION_PLAYER_VOLUME_UP );

    if ( event->key() == Qt::Key_Minus )
        cmdAction( ACTION_PLAYER_VOLUME_DOWN );

    if ( event->key() == Qt::Key_O )
    {
        SongEnqueueDialog dlg;

        if ( dlg.exec() == QDialog::Accepted )
            emit queueAdd( dlg.leFile->text(), dlg.leSinger->text() );
    }

    if ( event->key() == Qt::Key_S )
    {
        SongSearchDialog dlg;

        if ( dlg.exec() == QDialog::Accepted )
        {
            QString path = pSongDatabase->pathForId( dlg.selectedSongId() );

            if ( path.isEmpty() )
                return;

            QString singer = QInputDialog::getText( 0, "Enter singer name", "Singer name:" );
            emit queueAdd( path, singer );
        }
    }

    if ( event->key() == Qt::Key_N || event->key() == Qt::Key_Up )
        cmdAction( ACTION_QUEUE_NEXT );

    if ( event->key() == Qt::Key_P || event->key() == Qt::Key_Down )
        cmdAction( ACTION_QUEUE_PREVIOUS );

    if ( event->key() == Qt::Key_Z )
        cmdAction( ACTION_QUEUE_CLEAR );

}
