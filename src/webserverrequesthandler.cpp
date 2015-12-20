#include <QDebug>
#include "webserverrequesthandler.h"
#include <QThread>

WebServerRequestHandler::WebServerRequestHandler(QObject *parent) :
    QObjectHandler(parent)
{
}


QVariantMap WebServerRequestHandler::search(const QVariantMap &params)
{
    qDebug() << "search" << params;

    QVariantMap data;
    QThread::sleep( 20 );
    data.insert("found", "23" );
    return data;
}

QVariantMap WebServerRequestHandler::addsong(const QVariantMap &params)
{
    qDebug() << "addsong" << params;
    QThread::sleep( 20 );
    return QVariantMap();
}
