#include <QDBusConnection>
#include <QApplication>
#include <QMetaClassInfo>

#include "eventcontroller.h"
#include "eventcontroller_dbus.h"

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

void EventController::playerSongFinished(QString fileName)
{

}

void EventController::playerSongFailed(QString fileName)
{

}

void EventController::keyEvent(QKeyEvent *event)
{

}

void EventController::lircEvent(QString event)
{

}

void EventController::dbusEvent(QString event)
{

}
