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

#include <QNetworkInterface>

#include "libqhttpengine/QHttpEngine/QHttpServer"
#include "libqhttpengine/QHttpEngine/QFilesystemHandler"

#include "actionhandler_webserver.h"
#include "actionhandler_webserver_handler.h"
#include "mainwindow.h"
#include "settings.h"

WebServer * pWebServer;

ActionHandler_WebServer::ActionHandler_WebServer( QObject * parent )
    : QThread( parent )
{
}

void ActionHandler_WebServer::run()
{
    ActionHandler_WebServer_Handler serverHandler;

    connect( &serverHandler, SIGNAL(queueAdd(QString,QString,int)), pMainWindow, SLOT(queueAdd(QString,QString,int)), Qt::QueuedConnection );

    QHttpServer server( &serverHandler );

    // Attempt to listen on the specified port
    if ( !server.listen( QHostAddress::Any, pSettings->httpListenPort ) )
    {
        qCritical("Unable to listen on the specified port.");
        return;
    }

    // Socket-based stuff uses its own loop
    exec();
}


QString ActionHandler_WebServer::webURL() const
{
    // Enumerate all local IP addresses
    foreach ( const QHostAddress &address, QNetworkInterface::allAddresses() )
    {
        if ( address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost) )
        {
            return QString( "http://%1:%2" ).arg( address.toString() ) .arg( pSettings->httpListenPort );
        }
    }

    return QString::null;
}
