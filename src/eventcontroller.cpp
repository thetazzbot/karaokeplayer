#include <QDBusConnection>
#include <QApplication>
#include <QMetaClassInfo>

#include "eventcontroller.h"
#include "eventcontroller_dbus.h"
#include "songenqueuedialog.h"

EventController * pController;

EventController::EventController()
    : QObject()
{
    new EventController_DBus( this );

    if ( !QDBusConnection::sessionBus().registerObject( "/", this ) || !QDBusConnection::sessionBus().registerService( DBUS_SERVICE_NAME ) )
        qWarning("Cannot register dbus object");
}

EventController::~EventController()
{
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

}

void EventController::keyEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_Space )
        emit playerPauseResume();

    if ( event->key() == Qt::Key_Escape )
        emit playerStop();

    if ( event->key() == Qt::Key_Enter )
        emit playerStart();

    if ( event->key() == Qt::Key_Left )
        emit playerBackward();

    if ( event->key() == Qt::Key_Right )
        emit playerForward();

    if ( event->key() == Qt::Key_O )
    {
        SongEnqueueDialog dlg;

        if ( dlg.exec() == QDialog::Accepted )
            emit queueAdd( dlg.leFile->text(), dlg.leSinger->text() );
    }

    if ( event->key() == Qt::Key_N || event->key() == Qt::Key_Up )
        emit queueNext();

    if ( event->key() == Qt::Key_P || event->key() == Qt::Key_Down )
        emit queuePrevious();

    if ( event->key() == Qt::Key_Z )
        emit queueClear();
}

void EventController::lircEvent(QString event)
{

}

void EventController::dbusEvent(QString event)
{

}
