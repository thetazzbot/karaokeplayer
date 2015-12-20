#ifndef WEBSERVERREQUESTHANDLER_H
#define WEBSERVERREQUESTHANDLER_H

#include <QJsonDocument>
#include "libqhttpengine/QHttpEngine/QObjectHandler"

class EventController_WebServer_Handler : public QObjectHandler
{
    Q_OBJECT

    public:
        EventController_WebServer_Handler( QObject *parent = 0 );

        void    process( QHttpSocket *socket, const QString &path );

    signals:
        void    queueAdd( QString path, QString singer );

    public Q_SLOTS:
        void    onReadChannelFinished();

    private:
        void    handle(QHttpSocket *socket);
        bool    search( QHttpSocket *socket, QJsonDocument& document );
        bool    addsong( QHttpSocket *socket, QJsonDocument& document);

        void    sendData(QHttpSocket *socket, const QByteArray& data, const QByteArray &type = "application/json" );
};


#endif // WEBSERVERREQUESTHANDLER_H
