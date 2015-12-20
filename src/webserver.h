#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QThread>

class QHttpServer;
class QFilesystemHandler;
class WebServerRequestHandler;

class WebServer : public QObject
{
    Q_OBJECT

    public:
        WebServer();


        bool    init();

    private:
        QThread         m_webThread;
        QHttpServer *   m_server;
        QFilesystemHandler  * m_handler;
        WebServerRequestHandler * m_serverHandler;
};


extern WebServer * pWebServer;

#endif // WEBSERVER_H
