#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QThread>
/*
class QHttpServer;
class QFilesystemHandler;
class WebServerRequestHandler;
*/
class WebServer : public QThread
{
    Q_OBJECT

    public:
        WebServer();

    private:
        void run();
};


extern WebServer * pWebServer;

#endif // WEBSERVER_H
