#include "libqhttpengine/QHttpEngine/QHttpServer"
#include "libqhttpengine/QHttpEngine/QFilesystemHandler"

#include "webserver.h"
#include "webserverrequesthandler.h"

WebServer * pWebServer;

WebServer::WebServer()
    : QThread()
{
}

void WebServer::run()
{
    // Build the hierarchy of handlers
    QFilesystemHandler handler(":/static");
    handler.addRedirect(QRegExp("^$"), "/index.html");

    WebServerRequestHandler serverHandler;
    handler.addSubHandler( QRegExp("api/"), &serverHandler );

    QHttpServer server( &handler );

    // Attempt to listen on the specified port
    if ( !server.listen( QHostAddress::Any, 8000) )
    {
        qCritical("Unable to listen on the specified port.");
    }

    exec();
}
