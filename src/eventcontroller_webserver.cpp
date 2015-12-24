#include "libqhttpengine/QHttpEngine/QHttpServer"
#include "libqhttpengine/QHttpEngine/QFilesystemHandler"

#include "eventcontroller_webserver.h"
#include "eventcontroller_webserver_handler.h"
#include "mainwindow.h"
#include "settings.h"

WebServer * pWebServer;

EventController_WebServer::EventController_WebServer( QObject * parent )
    : QThread( parent )
{
}

void EventController_WebServer::run()
{
    EventController_WebServer_Handler serverHandler;

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
