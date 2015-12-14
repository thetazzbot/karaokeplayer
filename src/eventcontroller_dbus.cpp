#include "eventcontroller.h"
#include "eventcontroller_dbus.h"

EventController_DBus::EventController_DBus( QObject * parent )
    : QDBusAbstractAdaptor( parent )
{
}

void EventController_DBus::queueSong(QString song, QString singer)
{
    pController->queueAdd( song, singer );
}
