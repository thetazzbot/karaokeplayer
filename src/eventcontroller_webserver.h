#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QThread>

// A built-in web server running in a dedicated thread (so it doesn't block our main thread)
class EventController_WebServer : public QThread
{
    Q_OBJECT

    public:
        EventController_WebServer( QObject * parent );

    private:
        void run();
};

#endif // WEBSERVER_H
