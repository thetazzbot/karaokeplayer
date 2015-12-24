#include <QFile>
#include <QDir>
#include <QUuid>
#include <QVariant>
#include <QTextStream>

#include "eventcontroller.h"
#include "settings.h"
#include "songdatabase.h"
#include "util.h"
#include "sqlite3/sqlite3.h"

SongDatabase * pSongDatabase;


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
        pController->error( QString("Error opening sqlite database: %1") .arg( sqlite3_errmsg(m_sqlitedb) ) );

        return false;
    }

    // Create tables/indexes if not there
    if ( !execute( "CREATE TABLE IF NOT EXISTS songs( id INTEGER PRIMARY KEY, path TEXT, artist TEXT, title TEXT,"
                  "type TEXT, search TEXT, played INT, lastplayed INT, lyricdelay INT, data BLOB ); "
                  "CREATE INDEX IF NOT EXISTS idxSearch ON songs(search);"
                  "CREATE TABLE IF NOT EXISTS settings( version INTEGER, identifier TEXT, lastupdated INT, pathprefix TEXT );" ) )
        return false;

    // Verify/update version
    QMap<QString, QVariant> verinfo;

    if ( !queryRow( "SELECT * FROM settings", verinfo ) || verinfo.isEmpty() )
    {
        m_identifier = QUuid::createUuid().toString().mid( 1, 36 );
        m_currentVersion = 1;
        m_lastUpdate = time( 0 );

        // Set the settings
        if ( !execute( QString("INSERT INTO settings VALUES( %1, '%2', %3, '');") .arg( m_currentVersion) .arg(m_identifier) .arg(m_lastUpdate) ) )
            return false;
    }
    else
    {
        m_filePrefix = verinfo["pathprefix"].toString();
        m_identifier = verinfo["identifier"].toString();
        m_lastUpdate = verinfo["lastupdated"].toLongLong();
        m_currentVersion = verinfo["version"].toInt();
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

        Record rec;

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

        if ( !execute( QString("INSERT OR REPLACE INTO songs VALUES( %1, '%2', '%3', '%4', '%5', '%6', 0, 0, 0, NULL )")
                       .arg( id )
                       .arg( quote(path) )
                       .arg( quote(artist) )
                       .arg( quote(title) )
                       .arg( quote(type) )
                       .arg( quote(search) ) ) )
            continue;
    }

    QString prefix = pathPrefix;

    if ( !prefix.endsWith( QDir::separator() ) )
        prefix.append( QDir::separator() );

    execute( QString("UPDATE settings SET pathprefix='%1'") .arg( quote(prefix) ) );
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
    QMap<QString, QVariant> row;

    if ( queryRow( QString("SELECT * FROM songs WHERE id=%1") .arg(id), row ) )
        return m_filePrefix + row["path"].toString();

    return "";
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

bool SongDatabase::execute(const QString &sql)
{
    // Create tables/indexes if not there
    char * errmsg;

    if ( sqlite3_exec( m_sqlitedb, qPrintable(sql), 0, 0, &errmsg ) != 0 )
    {
        pController->error( QString("Error executing database command: %1\n%2").arg( errmsg ) .arg(sql) );
        return false;
    }

    return true;
}

QString& SongDatabase::quote(QString &orig)
{
    orig.replace( "'", "''" );
    return orig;
}

QString SongDatabase::quote(const QString &orig)
{
    QString newstr = orig;
    return newstr.replace( "'", "''" );
}


bool SongDatabase::search(const QString &substr, QList<SongDatabase::SearchResult> &results, unsigned int limit)
{
    sqlite3_stmt *stmt;
    results.clear();

    QString strsearch = substr.toUpper();
    QString query = QString( "SELECT id,artist,title,type FROM songs WHERE search LIKE '%%%1%%' LIMIT %2") .arg( quote(strsearch) ) .arg(limit);

    if ( sqlite3_prepare_v2( m_sqlitedb, qPrintable(query), -1, &stmt, 0 ) != SQLITE_OK )
        return false;

    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        SearchResult res;

        res.id = sqlite3_column_int64( stmt, 0 );
        res.artist = QString::fromUtf8( (const char*) sqlite3_column_text( stmt, 1 ), sqlite3_column_bytes( stmt, 1 ) );
        res.title = QString::fromUtf8( (const char*) sqlite3_column_text( stmt, 2 ), sqlite3_column_bytes( stmt, 2 ) );

        results.append( res );

        if ( --limit == 0 )
            break;
    }

    sqlite3_finalize( stmt );
    return !results.empty();
}
