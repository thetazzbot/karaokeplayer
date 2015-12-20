#include "libqhttpengine/QHttpEngine/QHttpServer"
#include "libqhttpengine/QHttpEngine/QFilesystemHandler"

#include "webserver.h"
#include "webserverrequesthandler.h"

WebServer * pWebServer;

WebServer::WebServer()
    : QObject()
{
    moveToThread( &m_webThread );

    // Build the hierarchy of handlers
    m_handler = new QFilesystemHandler(":/static");
    m_handler->addRedirect(QRegExp("^$"), "/index.html");

    m_serverHandler = new WebServerRequestHandler();
    m_handler->addSubHandler( QRegExp("api/"), m_serverHandler );

    m_server = new QHttpServer( m_handler );

    // Attempt to listen on the specified port
    if ( !m_server->listen( QHostAddress::Any, 8000) )
    {
        qCritical("Unable to listen on the specified port.");
    }

    m_webThread.start();
}
