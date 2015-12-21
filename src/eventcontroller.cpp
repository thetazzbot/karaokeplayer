#include <QDBusConnection>
#include <QApplication>
#include <QInputDialog>

#include "eventcontroller.h"
#include "eventcontroller_dbus.h"
#include "songenqueuedialog.h"
#include "songsearchdialog.h"
#include "songdatabase.h"
#include "settings.h"

#if defined (HAS_LIRC_SUPPORT)
    #include "eventcontroller_lirc.h"
#endif

#if defined (HAS_HTTP_SUPPORT)
    #include "eventcontroller_webserver.h"
#endif


EventController * pController;

EventController::EventController()
    : QObject()
{
    m_webserver = 0;
}

EventController::~EventController()
{
}

void EventController::start()
{
#if defined (HAS_DBUS_SUPPORT)
    new EventController_DBus( this );

    if ( !QDBusConnection::sessionBus().registerObject( "/", this ) || !QDBusConnection::sessionBus().registerService( DBUS_SERVICE_NAME ) )
        qWarning("Cannot register dbus object");
#endif

#if defined (HAS_LIRC_SUPPORT)
    if ( !pSettings->lircDevicePath.isEmpty()  )
    {
        EventController_LIRC * lirc = new EventController_LIRC( this );

        connect( lirc, SIGNAL(lircEvent(int)), this, SLOT(cmdEvent(int)) );
    }
#endif

#if defined (HAS_HTTP_SUPPORT)
    if ( pSettings->httpListenPort > 0 )
    {
        m_webserver = new EventController_WebServer( this );
        m_webserver->start();
    }
#endif
}

void EventController::stop()
{
#if defined (HAS_HTTP_SUPPORT)
    if ( m_webserver )
        m_webserver->exit( 0 );
#endif
}

int EventController::eventByName(const char *eventname)
{
    if ( !strcmp( eventname, "START" ) )
        return EVENT_PLAYER_START;

    if ( !strcmp( eventname, "PAUSE" ) )
        return EVENT_PLAYER_PAUSERESUME;

    if ( !strcmp( eventname, "STOP" ) )
        return EVENT_PLAYER_STOP;

    if ( !strcmp( eventname, "BACKWARD" ) )
        return EVENT_PLAYER_BACKWARD;

    if ( !strcmp( eventname, "FORWARD" ) )
        return EVENT_PLAYER_FORWARD;

    if ( !strcmp( eventname, "NEXT" ) )
        return EVENT_QUEUE_NEXT;

    if ( !strcmp( eventname, "PREVIOUS" ) )
        return EVENT_QUEUE_PREVIOUS;

    if ( !strcmp( eventname, "CLEAR" ) )
        return EVENT_QUEUE_CLEAR;

    return -1;
}

void EventController::playerSongFinished()
{
    emit queueNext();
}

void EventController::playerSongFailed()
{
    emit queueNext();
}

void EventController::error(QString message)
{
    qDebug("ERROR: %s", qPrintable(message));
}

void EventController::warning(QString message)
{
    qDebug("WARNING: %s", qPrintable(message));
}

bool EventController::cmdEvent(int event )
{
    switch ( event )
    {
    case EVENT_PLAYER_START:
        emit playerStart();
        break;

    case EVENT_PLAYER_PAUSERESUME:
        emit playerPauseResume();
        break;

    case EVENT_PLAYER_STOP:
        emit playerStop();
        break;

    case EVENT_PLAYER_BACKWARD:
        emit playerBackward();
        break;

    case EVENT_PLAYER_FORWARD:
        emit playerForward();
        break;

    case EVENT_QUEUE_NEXT:
        emit queueNext();
        break;

    case EVENT_QUEUE_PREVIOUS:
        emit queuePrevious();
        break;

    case EVENT_QUEUE_CLEAR:
        emit queueClear();
        break;

    default:
        return false;
    }

    return true;
}

void EventController::keyEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_Space )
        cmdEvent( EVENT_PLAYER_PAUSERESUME );

    if ( event->key() == Qt::Key_Escape )
        cmdEvent( EVENT_PLAYER_STOP );

    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
        cmdEvent( EVENT_PLAYER_START );

    if ( event->key() == Qt::Key_Left )
        cmdEvent( EVENT_PLAYER_BACKWARD );

    if ( event->key() == Qt::Key_Right )
        cmdEvent( EVENT_PLAYER_FORWARD );

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
        cmdEvent( EVENT_QUEUE_NEXT );

    if ( event->key() == Qt::Key_P || event->key() == Qt::Key_Down )
        cmdEvent( EVENT_QUEUE_PREVIOUS );

    if ( event->key() == Qt::Key_Z )
        cmdEvent( EVENT_QUEUE_CLEAR );

}

void EventController::dbusEvent(QString event)
{
}
