#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

#include "eventcontroller_webserver_handler.h"
#include "songdatabase.h"
#include "eventcontroller.h"


EventController_WebServer_Handler::EventController_WebServer_Handler(QObject *parent) :
    QObjectHandler(parent)
{
}

void EventController_WebServer_Handler::process(QHttpSocket *socket, const QString & )
{
    // Only POST requests are accepted - reject any other methods but ensure
    // that the Allow header is set in order to comply with RFC 2616
    if ( socket->method() != "POST" && socket->method() != "GET" )
    {
        socket->setHeader("Allow", "POST");
        socket->writeError(QHttpSocket::MethodNotAllowed);
        return;
    }

    // Check to see if the socket has finished receiving all of the data yet
    // or not - if so, jump to invokeSlot(), otherwise wait for the
    // readChannelFinished() signal
    if ( socket->bytesAvailable() >= socket->contentLength() )
    {
        handle( socket );
    }
    else
    {
        // Wait until the data is retrieved
        connect( socket, SIGNAL(readChannelFinished()), this, SLOT(onReadChannelFinished()) );
    }
}

void EventController_WebServer_Handler::onReadChannelFinished()
{
    handle( qobject_cast<QHttpSocket*>(sender()) );
}

void EventController_WebServer_Handler::handle( QHttpSocket *socket )
{
    QString path = socket->path();

    if ( path.isEmpty() || path == "/" )
        path = "/index.html";

    if ( !path.startsWith( "/api" ) )
    {
        // Simple serving
        QString localpath = "/home/tim/work/my/karaokeplayer/html" + path;

        QFile f( localpath );

        if ( !f.open( QIODevice::ReadOnly ) )
        {
            qDebug() << path << " not found " << localpath;
            socket->writeError(QHttpSocket::NotFound);
            return;
        }

        QByteArray data = f.readAll();
        QByteArray type = "text/html";

        if ( path.endsWith( ".js" ) )
            type = "application/javascript";
        else if ( path.endsWith( ".css" ) )
            type = "text/css";

        sendData( socket, data, type );

        qDebug() << path << " served " << localpath;
        return;
    }

    // Attempt to decode the JSON from the socket
    QJsonParseError error;
    QByteArray rdata = socket->readAll();
    QJsonDocument document = QJsonDocument::fromJson(rdata, &error);

    // Ensure that the document is valid
    if(error.error != QJsonParseError::NoError)
    {
        socket->writeError(QHttpSocket::BadRequest);
        return;
    }

    bool res = false;

    // what do we need to call?
    if ( socket->path() == "/api/search" )
        res = search( socket, document );
    else if ( socket->path() == "/api/addsong" )
        res = addsong( socket, document );

    if ( !res )
    {
        socket->writeError(QHttpSocket::InternalServerError);
        return;
    }

    socket->close();
}


bool EventController_WebServer_Handler::search( QHttpSocket *socket, QJsonDocument& document )
{
    QJsonObject obj = document.object();

    if ( !obj.contains( "q" ) )
        return false;

    QList< SongDatabase::SearchResult > results;
    QJsonArray out;

    if ( pSongDatabase->search( obj["q"].toString(), results ) )
    {
        Q_FOREACH( const SongDatabase::SearchResult& res, results )
        {
            QJsonObject rec;

            rec[ "i" ] = res.id;
            rec[ "a" ] = res.artist;
            rec[ "t"] = res.title;

            out.append( rec );
        }
    }

    sendData( socket, QJsonDocument( out ).toJson() );
    socket->close();
    return true;
}

bool EventController_WebServer_Handler::addsong( QHttpSocket *socket, QJsonDocument& document )
{
    QJsonObject obj = document.object();

    if ( !obj.contains( "i" ) || !obj.contains( "s" ) )
        return false;

    QString singer = obj["s"].toString();
    QString path = pSongDatabase->pathForId( obj["i"].toInt() );

    if ( path.isEmpty() )
        return false;

    emit queueAdd( path, singer );

    QJsonObject out;
    out["result"] = 1;

    sendData( socket, QJsonDocument( out ).toJson() );
    socket->close();
    return true;
}

void EventController_WebServer_Handler::sendData(QHttpSocket *socket, const QByteArray &data, const QByteArray& type )
{
    socket->setHeader( "Content-Length", QByteArray::number( data.length() ) );
    socket->setHeader("Content-Type", type );
    socket->write(data);
}
