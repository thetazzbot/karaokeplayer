/**************************************************************************
 *  Ulduzsoft Karaoke PLayer - cross-platform desktop karaoke player      *
 *  Copyright (C) 2015-2016 George Yunaev, support@ulduzsoft.com          *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *																	      *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

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
        // File serving - first look up in wwwroot
        QFile f;

        if ( !pSettings->httpDocumentRoot.isEmpty() )
        {
            f.setFileName( pSettings->httpDocumentRoot + path );
            f.open( QIODevice::ReadOnly );
        }

        if ( !f.isOpen() )
        {
            // Now look up in resources
            f.setFileName( ":/html" + path );
            f.open( QIODevice::ReadOnly );
        }

        if ( !f.isOpen() )
        {
            if ( pSettings->httpDocumentRoot.isEmpty() )
                Logger::debug( "WebServer: requested path %s is not found in resources", qPrintable(path) );
            else
                Logger::debug( "WebServer: requested path %s is not found in resources or %s", qPrintable(path), qPrintable(pSettings->httpDocumentRoot) );

            socket->writeError(QHttpSocket::NotFound);
            return;
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

    QList< SongDatabaseInfo > results;
    QJsonArray out;

    if ( pSongDatabase->search( obj["q"].toString(), results ) )
    {
        Q_FOREACH( const SongDatabaseInfo& res, results )
        {
            QJsonObject rec;

            rec[ "i" ] = res.id;
            rec[ "a" ] = res.artist;
            rec[ "t"] = res.title;
            rec[ "y"] = res.type;
            rec[ "r"] = res.rating;

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

    SongDatabaseInfo info;

    if ( !pSongDatabase->songById( id, info ) || singer.isEmpty() )
    {
        Logger::debug("WebServer: failed to add song %d: %s", singer.isEmpty() ? "singer is empty" : "song not found" );

        QJsonObject out;
        out["result"] = 0;

        sendData( socket, QJsonDocument( out ).toJson() );
        socket->close();
        return true;
    }

    Logger::debug("WebServer: %s added song %d: %s", qPrintable(singer), id, qPrintable( info.filePath ) );
    emit queueAdd( singer, id );

    QJsonObject out;
    out["result"] = 1;
    out["title"] = info.title;
    out["artist"] = info.artist;

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
    socket->setHeader( "Content-Type", type );
    socket->setHeader( "Cache-Control", "max-age=0, no-cache" );
    socket->setHeader( "Expires", "Thu, 01 Jan 1970 00:00:01 GMT" );

    socket->write(data);
}
