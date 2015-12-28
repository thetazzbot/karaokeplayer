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

#include "actionhandler.h"
#include "actionhandler_dbus.h"


ActionHandler_DBus::ActionHandler_DBus( QObject * parent )
    : QDBusAbstractAdaptor( parent )
{
}

void ActionHandler_DBus::queueSong( QString singer, int id)
{
    pActionHandler->queueAdd( singer, id );
}

void ActionHandler_DBus::queueSong( QString singer, QString path )
{
    pActionHandler->queueAdd( singer, path );
}
