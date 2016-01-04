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

static const int CURRENT_DB_SCHEMA_VERSION = 1;

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
            QString query = "SELECT rowid, path, artist, title, type, played, strftime('%s', lastplayed), lyricdelay, strftime('%s', added), rating FROM songs ";
            return prepare( db, query + wheresql, args );
        }

        SongDatabaseInfo getRowSongInfo()
        {
            SongDatabaseInfo info;

            info.id = columnInt64( 0 );
            info.filePath = pSettings->replacePath( columnText( 1 ) );
            info.artist = columnText( 2 );
            info.title = columnText( 3 );
            info.type = columnText( 4 );
            info.playedTimes = columnInt( 5 );
            info.lastPlayed = columnInt64( 6 );
            info.lyricDelay = columnInt( 7 );
            info.added = columnInt64( 8 );
            info.rating = columnInt( 9 );
            info.language = SongDatabase::languageFromInt( columnInt( 10 ) );

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
    m_lastUpdate = 0;
    m_sqlitedb = 0;
    m_totalSongCount = 0;
}

SongDatabase::~SongDatabase()
{
    if ( m_sqlitedb )
        sqlite3_close_v2( m_sqlitedb );
}

bool SongDatabase::init()
{
    if ( !sqlite3_open_v2( pSettings->songdbFilename.toUtf8().data(),
                           &m_sqlitedb,
                           SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                           0 ) == SQLITE_OK )
    {
        pActionHandler->error( QString("Error opening sqlite database: %1") .arg( sqlite3_errmsg(m_sqlitedb) ) );

        return false;
    }

    // CREATE TABLE IF NOT EXISTS songs( path TEXT, artist TEXT, title TEXT, type TEXT, search TEXT, played INT, lastplayed INT, lyricdelay INT, added INT, rating INT, language INT
    if ( !execute( "CREATE TABLE IF NOT EXISTS songs( path TEXT, artist TEXT, title TEXT,"
                  "type TEXT, search TEXT, played INT, lastplayed INT, lyricdelay INT, added INT, rating INT, language INT )" )
         || !execute( "CREATE INDEX IF NOT EXISTS idxSearch ON songs(search)" )
         || !execute( "CREATE INDEX IF NOT EXISTS idxPath ON songs(path)" )
         || !execute( "CREATE TABLE IF NOT EXISTS settings( version INTEGER, identifier TEXT, lastupdated INT)" ) )
        return false;

    // Verify/update version
    if ( verifyDatabaseVersion() )
        return false;

    return true;
}

bool SongDatabase::songById(int id, SongDatabaseInfo &info)
{
    SQLiteStatement stmt;

    if ( !stmt.prepareSongQuery( m_sqlitedb, QString("WHERE rowid=%1") .arg(id) ) )
        return false;

    if ( stmt.step() != SQLITE_ROW )
        return false;

    info = stmt.getRowSongInfo();
    return true;
}

bool SongDatabase::songByPath(const QString &path, SongDatabaseInfo &info)
{
    SQLiteStatement stmt;

    if ( !stmt.prepareSongQuery( m_sqlitedb, "WHERE path=?", QStringList() << pSettings->replacePath( path ) ) )
        return false;

    if ( stmt.step() != SQLITE_ROW )
        return false;

    info = stmt.getRowSongInfo();
    return true;
}

void SongDatabase::updatePlayedSong(int id, int newdelay, int newrating)
{
    execute( QString("UPDATE songs SET lyricdelay=%1,played=played+1,lastplayed=DATETIME(),rating=%2 WHERE rowid=%3") .arg( newdelay) .arg(newrating) .arg(id) );
}

bool SongDatabase::browseInitials( QList<QChar>& artistInitials)
{
    artistInitials.clear();

    SQLiteStatement stmt;

    if ( !stmt.prepare( m_sqlitedb, "SELECT DISTINCT(SUBSTR(artist, 1, 1)) from songs ORDER BY artist") )
        return false;

    while ( stmt.step() == SQLITE_ROW )
        artistInitials.append( stmt.columnText( 0)[0] );

    return !artistInitials.empty();
}

bool SongDatabase::browseArtists(const QChar &artistInitial, QStringList &artists)
{
    artists.clear();

    SQLiteStatement stmt;

    if ( !stmt.prepare( m_sqlitedb, "SELECT DISTINCT(artist) FROM songs WHERE artist LIKE ? ORDER BY artist", QStringList() << QString("%1%%") .arg(artistInitial) ) )
        return false;

    while ( stmt.step() == SQLITE_ROW )
        artists.append( stmt.columnText(0) );

    return !artists.empty();
}

bool SongDatabase::browseSongs(const QString &artist, QList<SongDatabaseInfo> &results)
{
    results.clear();

    SQLiteStatement stmt;

    if ( !stmt.prepareSongQuery( m_sqlitedb, "WHERE artist=? ORDER BY title", QStringList() << artist ) )
        return false;

    while ( stmt.step() == SQLITE_ROW )
        results.append( stmt.getRowSongInfo() );

    return !results.empty();
}

QString SongDatabase::languageFromInt(unsigned int value)
{
    char buf[8];
    sprintf( buf, "%c%c%c", (value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF );

    return QString(buf);
}

bool SongDatabase::updateDatabase(const QList<SongDatabaseScanner::SongDatabaseEntry> entries)
{
    if ( !execute( "BEGIN TRANSACTION" ) )
        return false;

    Q_FOREACH( const SongDatabaseScanner::SongDatabaseEntry& e, entries )
    {
        // We use a separate search field since sqlite is not necessary built with full Unicode support (nor we want it to be)
        QString search = e.artist.toUpper() + " " + e.title.toUpper();

        QStringList params;
        params << e.filePath << e.artist << e.title << e.type << search;

        // path TEXT, artist TEXT, title TEXT type TEXT, search TEXT, played INT, lastplayed INT, lyricdelay INT, added INT, rating INT, language INT
        if ( !execute( QString("INSERT OR REPLACE INTO songs VALUES( ?, ?, ?, ?, ?, 0, 0, 0, DATETIME(), 0, %1 )").arg( e.language ), params ) )
        {
            execute( "ROLLBACK TRANSACTION" );
            return false;
        }
    }

    return execute( "COMMIT TRANSACTION" );
}

bool SongDatabase::updateLastScan()
{
    execute( "UPDATE settings SET lastupdated=DATETIME()" );
}

bool SongDatabase::clearDatabase()
{
    return execute( "DELETE FROM songs");
}

qint64 SongDatabase::lastDatabaseUpdate() const
{
    return m_lastUpdate;
}

unsigned int SongDatabase::getSongCount() const
{
    return m_totalSongCount;
}

bool SongDatabase::verifyDatabaseVersion()
{
    SQLiteStatement stmt;

    if ( !stmt.prepare( m_sqlitedb, "SELECT version,identifier,strftime('%s', lastupdated) FROM settings" ) || stmt.step() != SQLITE_ROW )
    {
        QString identifier = QUuid::createUuid().toString().mid( 1, 36 );
        m_lastUpdate = 0;

        // Set the settings
        if ( !execute( QString("INSERT INTO settings VALUES( %1, ?, 0)") .arg( CURRENT_DB_SCHEMA_VERSION ), QStringList() << identifier ) )
            return false;

        Logger::debug( "Initialized new karaoke song database at %s", qPrintable(pSettings->songdbFilename) );
    }
    else
    {
        //int currentVersion = stmt.columnInt64( 0 );
        // m_identifier = stmt.columnText( 1 );

        m_lastUpdate = stmt.columnInt64( 2 );
        Logger::debug( "Loaded existing karaoke song database from %s", qPrintable(pSettings->songdbFilename) );

        SQLiteStatement songstmt;

        if ( !songstmt.prepare( m_sqlitedb, "SELECT COUNT(rowid) FROM songs" ) )
        {
            pActionHandler->error( QString("Error calculating song count: %1").arg( sqlite3_errmsg( m_sqlitedb ) ) );
            return false;
        }

        // Eat all the results
        if ( songstmt.step() == SQLITE_ROW )
            m_totalSongCount = songstmt.columnInt( 0 );
    }

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

    // Eat all the results
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
