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

    connect( &serverHandler, SIGNAL(queueAdd(QString,QString,int)), pMainWindow, SLOT(queueAdd(QString,QString,int)) );

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
