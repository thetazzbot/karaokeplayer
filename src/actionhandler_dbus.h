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
