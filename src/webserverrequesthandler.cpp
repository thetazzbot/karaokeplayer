#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

#include "webserverrequesthandler.h"
#include "songdatabase.h"
#include "eventcontroller.h"


WebServerRequestHandler::WebServerRequestHandler(QObject *parent) :
    QObjectHandler(parent)
{
}

void WebServerRequestHandler::process(QHttpSocket *socket, const QString &)
{
    // Only POST requests are accepted - reject any other methods but ensure
    // that the Allow header is set in order to comply with RFC 2616
    if ( socket->method() != "POST")
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

void WebServerRequestHandler::onReadChannelFinished()
{
    handle( qobject_cast<QHttpSocket*>(sender()) );
}

void WebServerRequestHandler::handle( QHttpSocket *socket )
{
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

/*    // Convert the return value to JSON and write it to the socket
    QByteArray data = QJsonDocument(QJsonObject::fromVariantMap(retVal)).toJson();
    socket->setHeader("Content-Length", QByteArray::number(data.length()));
    socket->setHeader("Content-Type", "application/json");
    socket->write(data);*/
    socket->close();
}


bool WebServerRequestHandler::search( QHttpSocket *socket, QJsonDocument& document )
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

bool WebServerRequestHandler::addsong( QHttpSocket *socket, QJsonDocument& document )
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

void WebServerRequestHandler::sendData(QHttpSocket *socket, const QByteArray &data)
{
    socket->setHeader( "Content-Length", QByteArray::number( data.length() ) );
    socket->setHeader("Content-Type", "application/json");
    socket->write(data);
}
