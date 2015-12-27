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

#include <QFile>
#include <QDir>
#include <QUuid>
#include <QVariant>
#include <QTextStream>

#include "actionhandler.h"
#include "settings.h"
#include "songdatabase.h"
#include "util.h"
#include "sqlite3.h"
#include "logger.h"

SongDatabase * pSongDatabase;

// A simple statement wrapper ensuring finalize() is called
class SQLiteStatement
{
    public:
        SQLiteStatement()   { stmt = 0; }
        ~SQLiteStatement() { if ( stmt ) sqlite3_finalize( stmt ); }

        bool prepare( sqlite3 * db, const QString& sql, const QStringList& args = QStringList() )
        {
            // Prepare the sql
            if ( sqlite3_prepare_v2( db, qPrintable(sql), -1, &stmt, 0 ) != SQLITE_OK )
                return false;

            // Bind values if there are any
            for ( int i = 0; i < args.size(); i++ )
            {
                m_args.push_back( args[i].toUtf8() );
                if ( sqlite3_bind_text( stmt, i + 1, m_args.last().data(), -1, 0 ) != SQLITE_OK )
                    return false;
            }

            return true;
        }

        qint64 columnInt64( int column )
        {
            return sqlite3_column_int64( stmt, column );
        }

        int columnInt( int column )
        {
            return sqlite3_column_int( stmt, column );
        }

        QString columnText( int column )
        {
            return QString::fromUtf8( (const char*) sqlite3_column_text( stmt, column ), sqlite3_column_bytes( stmt, column ) );
        }

        int step() { return sqlite3_step( stmt ); }

        // Two functions below must be in sync regarding the column order
        bool prepareSongQuery( sqlite3 * db, const QString& wheresql, const QStringList& args = QStringList() )
        {
            QString query = "SELECT id, path, artist, title, type, played, strftime('%s', lastplayed), lyricdelay, strftime('%s', added), rating FROM songs ";
            return prepare( db, query + wheresql, args );
        }

        SongDatabaseInfo getRowSongInfo()
        {
            SongDatabaseInfo info;

            info.id = columnInt64( 0 );
            info.filePath = pSettings->songPathPrefix + columnText( 1 );
            info.artist = columnText( 2 );
            info.title = columnText( 3 );
            info.type = columnText( 4 );
            info.playedTimes = columnInt( 5 );
            info.lastPlayed = columnInt64( 6 );
            info.lyricDelay = columnInt( 7 );
            info.added = columnInt64( 8 );
            info.rating = columnInt( 9 );

            return info;
        }

    public:
        sqlite3_stmt * stmt;

        // Stores UTF-8 strings of args as sqlite needs them to live until the statement is executed
        QList<QByteArray>   m_args;
};

SongDatabaseInfo::SongDatabaseInfo()
{
    id = 0;
    playedTimes = 0;
    lastPlayed = 0;
    lyricDelay = 0;
    added = 0;
    rating = 0;
}


SongDatabase::SongDatabase(QObject *parent)
    : QObject( parent )
{
    m_currentVersion = 1;
    m_lastUpdate = 0;
    m_sqlitedb = 0;
}

SongDatabase::~SongDatabase()
{
    if ( m_sqlitedb )
        sqlite3_close_v2( m_sqlitedb );
}

bool SongDatabase::init()
{
    if ( !sqlite3_open( pSettings->songdbFilename.toUtf8().data(), &m_sqlitedb ) == SQLITE_OK )
    {
        pActionHandler->error( QString("Error opening sqlite database: %1") .arg( sqlite3_errmsg(m_sqlitedb) ) );

        return false;
    }

    // CREATE TABLE IF NOT EXISTS songs( id INTEGER PRIMARY KEY, path TEXT, artist TEXT, title TEXT, type TEXT, search TEXT, played INT, lastplayed INT, lyricdelay INT, added INT, rating INT
    // Create tables/indexes if not there
    if ( !execute( "CREATE TABLE IF NOT EXISTS songs( id INTEGER PRIMARY KEY, path TEXT, artist TEXT, title TEXT,"
                  "type TEXT, search TEXT, played INT, lastplayed INT, lyricdelay INT, added INT, rating INT )" )
         || !execute( "CREATE INDEX IF NOT EXISTS idxSearch ON songs(search)" )
         || !execute( "CREATE TABLE IF NOT EXISTS settings( version INTEGER, identifier TEXT, lastupdated INT)" ) )
        return false;

    // Verify/update version
    SQLiteStatement stmt;

    if ( !stmt.prepare( m_sqlitedb, "SELECT version,identifier,lastupdated FROM settings" ) || stmt.step() != SQLITE_ROW )
    {
        m_identifier = QUuid::createUuid().toString().mid( 1, 36 );
        m_currentVersion = 1;
        m_lastUpdate = time( 0 );

        // Set the settings
        if ( !execute( QString("INSERT INTO settings VALUES( %1, ?, %2)") .arg( m_currentVersion) .arg(m_lastUpdate), QStringList() << m_identifier ) )
            return false;

        Logger::debug( "Initialized new karaoke song database at %s", qPrintable(pSettings->songdbFilename) );
    }
    else
    {
        m_currentVersion = stmt.columnInt64( 0 );
        m_identifier = stmt.columnText( 1 );
        m_lastUpdate = stmt.columnInt64( 2 );

        Logger::debug( "Loaded existing karaoke song database at %s", qPrintable(pSettings->songdbFilename) );
    }

    //importFromText( "karaoke.text", "/mnt/karaoke" );
    return true;
}


bool SongDatabase::importFromText(const QString &filename, const QString& pathPrefix )
{
    QFile fin( filename );

    if ( !fin.open( QIODevice::ReadOnly ) )
        return false;

    execute( "BEGIN TRANSACTION" );

    while ( !fin.atEnd() )
    {
        QByteArray bline = fin.readLine();

        if ( bline.isEmpty() )
            break;

        if ( bline.trimmed().isEmpty() || bline[0] == '#' )
            continue;

        QString line = QString::fromUtf8( bline );

        // ID type file
        int o = Util::indexOfSpace( line );

        if ( o == -1 )
            continue;

        QString id = line.left( o );
        QString type = line.mid( o ).trimmed();

        if ( (o = Util::indexOfSpace( type )) == -1 )
            continue;

        QString artist, title, path = type.mid( o + 1 ).trimmed();
        type = type.left( o );

        // Split to artist/title
        if ( !pathToArtistTitle( path, artist, title ) )
            abort();

        QString search = artist.toUpper() + " " + title.toUpper();

        QStringList params;
        params << path << artist << title << type << search;

        // id INTEGER PRIMARY KEY, path TEXT, artist TEXT, title TEXT type TEXT, search TEXT, played INT, lastplayed INT, lyricdelay INT, data BLOB
        if ( !execute( QString("INSERT OR REPLACE INTO songs VALUES( %1, ?, ?, ?, ?, ?, 0, 0, 0, 0, DATETIME(), NULL )").arg( id ), params ) )
            continue;
    }

    QString prefix = pathPrefix;

    if ( !prefix.endsWith( QDir::separator() ) )
        prefix.append( QDir::separator() );


    execute( "UPDATE settings SET pathprefix=?", QStringList() << prefix );
    execute( "COMMIT" );

    return true;
}


bool SongDatabase::songById(int id, SongDatabaseInfo &info)
{
    SQLiteStatement stmt;

    if ( !stmt.prepareSongQuery( m_sqlitedb, QString("WHERE id=%1") .arg(id) ) )
        return false;

    if ( stmt.step() != SQLITE_ROW )
        return false;

    info = stmt.getRowSongInfo();
    return true;
}

void SongDatabase::updatePlayedSong(int id, int newdelay, int newrating)
{
    execute( QString("UPDATE songs SET lyricdelay=%1,played=played+1,lastplayed=DATETIME(),rating=%2 WHERE id=%3") .arg( newdelay) .arg(newrating) .arg(id) );
}

bool SongDatabase::pathToArtistTitle(const QString &path, QString &artist, QString &title)
{
    int p = path.lastIndexOf( QDir::separator() );

    if ( p == -1 )
        return false;

    title = path.mid( p + 1 );
    artist = path.left( p );

    // Trim down the artist if needed - may not be there (i.e. "Adriano Celentano/Le tempo.mp3")
    if ( (p = artist.lastIndexOf( QDir::separator() )) != -1 )
        artist = artist.mid( p + 1 );

    // Trim the extension from title
    if ( (p = title.lastIndexOf( '.' )) != -1 )
        title = title.left( p );

    return true;
}

bool SongDatabase::execute( const QString &sql, const QStringList& args )
{
    SQLiteStatement stmt;

    if ( !stmt.prepare( m_sqlitedb, sql, args  ) )
    {
        pActionHandler->error( QString("Error executing database command: %1\n%2").arg( sqlite3_errmsg( m_sqlitedb ) ) .arg(sql) );
        return false;
    }

    int res;

    while ( (res = stmt.step()) == SQLITE_ROW )
        ;

    if ( res == SQLITE_DONE )
        return true;
    else
    {
        pActionHandler->error( QString("Error executing database command: %1\n%2").arg( sqlite3_errmsg( m_sqlitedb ) ) .arg(sql) );
        return false;
    }
}

bool SongDatabase::search(const QString &substr, QList<SongDatabaseInfo> &results, unsigned int limit)
{
    results.clear();

    SQLiteStatement stmt;

    if ( !stmt.prepareSongQuery( m_sqlitedb, "WHERE search LIKE ? ORDER BY artist,title", QStringList() << "%" + substr.toUpper() + "%" ) )
        return false;

    while ( stmt.step() == SQLITE_ROW )
    {
        SongDatabaseInfo info = stmt.getRowSongInfo();
        results.append( info );

        if ( --limit == 0 )
            break;
    }

    return !results.empty();
}
