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

    public:
        sqlite3_stmt * stmt;

        // Stores UTF-8 strings of args as sqlite needs them to live until the statement is executed
        QList<QByteArray>   m_args;
};


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

    // Create tables/indexes if not there
    if ( !execute( "CREATE TABLE IF NOT EXISTS songs( id INTEGER PRIMARY KEY, path TEXT, artist TEXT, title TEXT,"
                  "type TEXT, search TEXT, played INT, lastplayed INT, lyricdelay INT, added INT, data BLOB )" )
         || !execute( "CREATE INDEX IF NOT EXISTS idxSearch ON songs(search)" )
         || !execute( "CREATE TABLE IF NOT EXISTS settings( version INTEGER, identifier TEXT, lastupdated INT)" ) )
        return false;

    // Verify/update version
    SQLiteStatement stmt;

    if ( !stmt.prepare( m_sqlitedb, "SELECT version,identifier,lastupdated,pathprefix FROM settings" ) || stmt.step() != SQLITE_ROW )
    {
        m_identifier = QUuid::createUuid().toString().mid( 1, 36 );
        m_currentVersion = 1;
        m_lastUpdate = time( 0 );

        // Set the settings
        if ( !execute( QString("INSERT INTO settings VALUES( %1, ?, %2)") .arg( m_currentVersion) .arg(m_lastUpdate), QStringList() << m_identifier ) )
            return false;
    }
    else
    {
        m_currentVersion = stmt.columnInt64( 0 );
        m_identifier = stmt.columnText( 1 );
        m_lastUpdate = stmt.columnInt64( 2 );
    }

    //importFromText( "/home/tim/work/my/karaokeplayer/test/karaoke.text", "/mnt/karaoke" );
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
        if ( !execute( QString("INSERT OR REPLACE INTO songs VALUES( %1, ?, ?, ?, ?, ?, 0, 0, 0, DATETIME(), NULL )").arg( id ), params ) )
            continue;
    }

    QString prefix = pathPrefix;

    if ( !prefix.endsWith( QDir::separator() ) )
        prefix.append( QDir::separator() );


    execute( "UPDATE settings SET pathprefix=?", QStringList() << prefix );
    execute( "COMMIT" );

    return true;
}

/*
bool SongDatabase::exportToText(const QString &filename)
{
    QFile fout( filename );

    if ( !fout.open( QIODevice::WriteOnly ) )
        return false;

    QTextStream stream( &fout );
    stream.setCodec( "utf-8" );

    stream << "$IDENTIFIER " << m_identifier << "\n";
    stream << "$VERSION " << m_currentVersion << "\n";
    stream << "$LASTUPDATE " << m_lastUpdate << "\n";

    for ( QMap<int, Record>::const_iterator it = m_database.begin(); it != m_database.end(); ++it )
    {
        stream.setFieldWidth( 5 );
        stream.setPadChar( '0' );
        stream << it.key();
        stream.reset();
        stream << " " << it.value().type;
        stream << " " << it.value().filename;
        stream << "\n";
    }

    return true;
}
*/

QString SongDatabase::pathForId(int id)
{
    SQLiteStatement stmt;

    if ( !stmt.prepare( m_sqlitedb, QString("SELECT path FROM songs WHERE id=%1") .arg(id)) || stmt.step() != SQLITE_ROW )
        return "";

    return pSettings->songPathPrefix + stmt.columnText( 0 );
}

int SongDatabase::getLyricDelay(int id)
{
    SQLiteStatement stmt;

    if ( !stmt.prepare( m_sqlitedb, QString("SELECT lyricdelay FROM songs WHERE id=%1") .arg(id)) || stmt.step() != SQLITE_ROW )
        return 0;

    return stmt.columnInt( 0 );
}

void SongDatabase::updatePlayedSong(int id, int newdelay)
{
    execute( QString("UPDATE songs SET lyricdelay=%1,played=played+1,lastplayed=DATETIME() WHERE id=%2") .arg( newdelay) .arg(id) );
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

/*
bool SongDatabase::queryRow(const QString &sql, QMap<QString, QVariant> &output )
{
    sqlite3_stmt *stmt;

    if ( sqlite3_prepare_v2( m_sqlitedb, qPrintable(sql), -1, &stmt, 0 ) != SQLITE_OK )
    {
        pController->error( QString("Error querying sqlite database: %1").arg( sqlite3_errmsg(m_sqlitedb)) );
        return false;
    }

    int res = sqlite3_step( stmt );

    if ( res == SQLITE_DONE )
    {
        sqlite3_finalize( stmt );
        return true;
    }

    if ( res != SQLITE_ROW )
    {
        pController->error( QString("Error querying sqlite database: %1").arg( sqlite3_errmsg(m_sqlitedb)) );
        sqlite3_finalize( stmt );
        return false;
    }

    for ( int i = 0; i < sqlite3_column_count( stmt ); i++ )
    {
        const char * colname = sqlite3_column_name( stmt, i );

        if ( colname == 0 )
        {
            pController->error( QString("Error querying sqlite column name: %1").arg( sqlite3_errmsg(m_sqlitedb)) );
            sqlite3_finalize( stmt );
            return false;
        }

        QVariant value;

        switch ( sqlite3_column_type( stmt, i ) )
        {
        case SQLITE_INTEGER:
            value.setValue( sqlite3_column_int64( stmt, i ) );
            break;

        case SQLITE_FLOAT:
            value.setValue( sqlite3_column_double( stmt, i ) );
            break;

        case SQLITE_TEXT:
            value.setValue( QString::fromUtf8( (const char*) sqlite3_column_text( stmt, i ), sqlite3_column_bytes( stmt, i ) ) );
            break;

        case SQLITE_BLOB:
            value.setValue( QByteArray( (const char*) sqlite3_column_blob( stmt, i ), sqlite3_column_bytes( stmt, i ) ) );
            break;

        case SQLITE_NULL:
            break;
        }

        output[ QString::fromUtf8( colname ) ] = value;
    }

    sqlite3_finalize( stmt );
    return true;
}
*/
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

bool SongDatabase::search(const QString &substr, QList<SongDatabase::SearchResult> &results, unsigned int limit)
{
    results.clear();

    SQLiteStatement stmt;

    if ( !stmt.prepare( m_sqlitedb, "SELECT id,artist,title,type FROM songs WHERE search LIKE ? ORDER BY artist,title", QStringList() << "%" + substr.toUpper() + "%" ) )
        return false;

    while ( stmt.step() == SQLITE_ROW )
    {
        SearchResult res;

        res.id = stmt.columnInt64( 0 );
        res.artist = stmt.columnText( 1 );
        res.title = stmt.columnText( 2 );

        results.append( res );

        if ( --limit == 0 )
            break;
    }

    return !results.empty();
}
