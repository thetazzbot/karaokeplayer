#include <QMimeDatabase>
#include <QMimeType>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

#include "settings.h"
#include "logger.h"
#include "actionhandler_webserver_handler.h"
#include "songdatabase.h"
#include "songqueue.h"
#include "actionhandler.h"


ActionHandler_WebServer_Handler::ActionHandler_WebServer_Handler(QObject *parent) :
    QObjectHandler(parent)
{
}

void ActionHandler_WebServer_Handler::process(QHttpSocket *socket, const QString & )
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

void ActionHandler_WebServer_Handler::onReadChannelFinished()
{
    handle( qobject_cast<QHttpSocket*>(sender()) );
}

void ActionHandler_WebServer_Handler::handle( QHttpSocket *socket )
{
    QString path = socket->path();

    if ( path.isEmpty() || path == "/" )
        path = "/index.html";

    if ( !path.startsWith( "/api" ) )
    {
        // File serving - first look up in resources
        QString localpath = ":/html" + path;
        QFile f( localpath );

        if ( !f.open( QIODevice::ReadOnly ) )
        {
            // Attempt to serve from wwwroot if defined
            if ( !pSettings->httpDocumentRoot.isEmpty() )
            {
                f.setFileName( pSettings->httpDocumentRoot + path );

                if ( !f.open( QIODevice::ReadOnly ) )
                {
                    Logger::debug( "WebServer: requested path %s is not found in resources or %s", qPrintable(path), qPrintable(pSettings->httpDocumentRoot) );
                    socket->writeError(QHttpSocket::NotFound);
                    return;
                }
            }
            else
            {
                Logger::debug( "WebServer: requested path %s is not found in resources, and httpDocumentRoot is not defined", qPrintable(path) );
                socket->writeError(QHttpSocket::NotFound);
                return;
            }
        }

        QByteArray data = f.readAll();
        QByteArray type = QMimeDatabase().mimeTypeForFile( f.fileName() ).name().toUtf8();

        Logger::debug( "WebServer: serving content %s from %s, type %s", qPrintable(path), f.fileName().startsWith( ':' ) ? "resources" : qPrintable(f.fileName()), qPrintable(type) );
        sendData( socket, data, type );
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
    else if ( socket->path() == "/api/listqueue" )
        res = listqueue( socket, document );

    if ( !res )
    {
        socket->writeError(QHttpSocket::InternalServerError);
        return;
    }

    socket->close();
}


bool ActionHandler_WebServer_Handler::search( QHttpSocket *socket, QJsonDocument& document )
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

bool ActionHandler_WebServer_Handler::addsong( QHttpSocket *socket, QJsonDocument& document )
{
    QJsonObject obj = document.object();

    if ( !obj.contains( "i" ) || !obj.contains( "s" ) )
        return false;

    int id = obj["i"].toInt();
    QString singer = obj["s"].toString();
    QString path = pSongDatabase->pathForId( id );

    if ( path.isEmpty() || singer.isEmpty() )
    {
        Logger::debug("WebServer: failed to add song %d: %s", path.isEmpty() ? "path not found" : "singer is empty");

        QJsonObject out;
        out["result"] = 0;

        sendData( socket, QJsonDocument( out ).toJson() );
        socket->close();
        return true;
    }

    Logger::debug("WebServer: %s added song %d: %s", qPrintable(singer), id, qPrintable(path) );
    emit queueAdd( path, singer, id );

    QJsonObject out;
    out["result"] = 1;

    sendData( socket, QJsonDocument( out ).toJson() );
    socket->close();
    return true;
}

bool ActionHandler_WebServer_Handler::listqueue(QHttpSocket *socket, QJsonDocument &)
{
    QList<SongQueue::Song> queue;

    // Get the song list
    pSongQueue->exportQueue( queue );

    QJsonArray out;

    Q_FOREACH( const SongQueue::Song& s, queue )
    {
        QJsonObject rec;

        rec[ "i" ] = s.id;
        rec[ "p" ] = s.singer;
        rec[ "t"] = s.title;
        rec[ "s"] = s.stateText();

        out.append( rec );
    }

    sendData( socket, QJsonDocument( out ).toJson() );
    socket->close();
    return true;
}


void ActionHandler_WebServer_Handler::sendData(QHttpSocket *socket, const QByteArray &data, const QByteArray& type )
{
    socket->setHeader( "Content-Length", QByteArray::number( data.length() ) );
    socket->setHeader("Content-Type", type );
    socket->write(data);
}
