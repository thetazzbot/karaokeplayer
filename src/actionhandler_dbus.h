#ifndef EVENTCONTROLLER_DBUS_H
#define EVENTCONTROLLER_DBUS_H

#include <QDBusAbstractAdaptor>

#define DBUS_SERVICE_NAME   "com.ulduzsoft.dbus.KaraokePlayer"

class ActionHandler_DBus : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO( "D-Bus Interface", DBUS_SERVICE_NAME )

    public:
        ActionHandler_DBus( QObject * parent );

    public slots:
        void queueSong( QString song, QString singer, int id );
        void queueSong( QString song, QString singer );
};

#endif // EVENTCONTROLLER_DBUS_H
