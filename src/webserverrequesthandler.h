#ifndef WEBSERVERREQUESTHANDLER_H
#define WEBSERVERREQUESTHANDLER_H

#include <QVariantMap>
#include "libqhttpengine/QHttpEngine/QObjectHandler"

class WebServerRequestHandler : public QObjectHandler
{
    Q_OBJECT

    public:
        WebServerRequestHandler(QObject *parent = 0);

    public Q_SLOTS:
        QVariantMap search(const QVariantMap &params);
        QVariantMap addsong(const QVariantMap &params);

};

#endif // WEBSERVERREQUESTHANDLER_H
