#include "actionhandler.h"
#include "actionhandler_dbus.h"


ActionHandler_DBus::ActionHandler_DBus( QObject * parent )
    : QDBusAbstractAdaptor( parent )
{
}

void ActionHandler_DBus::queueSong(QString song, QString singer)
{
    pActionHandler->queueAdd( song, singer );
}
