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

#ifndef WEBSERVERREQUESTHANDLER_H
#define WEBSERVERREQUESTHANDLER_H

#include <QJsonDocument>
#include "libqhttpengine/QHttpEngine/QObjectHandler"

class ActionHandler_WebServer_Handler : public QObjectHandler
{
    Q_OBJECT

    public:
        ActionHandler_WebServer_Handler( QObject *parent = 0 );

        void    process( QHttpSocket *socket, const QString &path );

    signals:
        void    queueAdd( QString singer, int id );

    public Q_SLOTS:
        void    onReadChannelFinished();

    private:
        void    handle(QHttpSocket *socket);
        bool    search( QHttpSocket *socket, QJsonDocument& document );
        bool    addsong( QHttpSocket *socket, QJsonDocument& document);
        bool    listqueue( QHttpSocket * socket, QJsonDocument& document );
        bool    listDatabase( QHttpSocket * socket, QJsonDocument& document );

        void    sendData(QHttpSocket *socket, const QByteArray& data, const QByteArray &type = "application/json" );
};


#endif // WEBSERVERREQUESTHANDLER_H
