#ifndef WEBSERVERREQUESTHANDLER_H
#define WEBSERVERREQUESTHANDLER_H

#include <QJsonDocument>
#include "libqhttpengine/QHttpEngine/QObjectHandler"

class WebServerRequestHandler : public QObjectHandler
{
    Q_OBJECT

    public:
        WebServerRequestHandler(QObject *parent = 0);

        void    process( QHttpSocket *socket, const QString &path );

    signals:
        void    queueAdd( QString path, QString singer );

    public Q_SLOTS:
        void    onReadChannelFinished();

    private:
        void    handle(QHttpSocket *socket);
        bool    search( QHttpSocket *socket, QJsonDocument& document );
        bool    addsong( QHttpSocket *socket, QJsonDocument& document);

        void    sendData( QHttpSocket *socket, const QByteArray& data );
};


#endif // WEBSERVERREQUESTHANDLER_H
