#include "libqhttpengine/QHttpEngine/QHttpServer"
#include "libqhttpengine/QHttpEngine/QFilesystemHandler"

#include "webserver.h"
#include "webserverrequesthandler.h"
#include "mainwindow.h"

WebServer * pWebServer;

WebServer::WebServer()
    : QThread()
{
}

void WebServer::run()
{
    // Build the hierarchy of handlers
//    QFilesystemHandler handler(":/static");
//    handler.addRedirect(QRegExp("^$"), "/index.html");

    WebServerRequestHandler serverHandler;
    serverHandler.addRedirect(QRegExp("^$"), "/index.html");
    //handler.addSubHandler( QRegExp("api/"), &serverHandler );

    connect( &serverHandler, SIGNAL(queueAdd(QString,QString)), pMainWindow, SLOT(queueAdd(QString,QString)) );

    QHttpServer server( &serverHandler );

    // Attempt to listen on the specified port
    if ( !server.listen( QHostAddress::Any, 8000) )
    {
        qCritical("Unable to listen on the specified port.");
        return;
    }

    exec();
}
