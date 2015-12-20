#include <QFile>
#include <QDir>
#include <QUuid>
#include <QTextStream>

#include "settings.h"
#include "songdatabase.h"

SongDatabase * pSongDatabase;


SongDatabase::SongDatabase()
{
    m_identifier = QUuid::createUuid().toString().mid( 1, 36 );
    m_currentVersion = 1;
    m_lastUpdate = time( 0 );
}

bool SongDatabase::init()
{
    QFile fin( pSettings->songdbFilename );

    if ( !fin.open( QIODevice::ReadOnly ) )
        return false;

    while ( !fin.atEnd() )
    {
        QByteArray bline = fin.readLine();

        if ( bline.isEmpty() )
            break;

        if ( bline.trimmed().isEmpty() || bline[0] == '#' )
            continue;

        QString line = QString::fromUtf8( bline );

        // System field?
        if ( line.startsWith( '$' ) )
        {
            int o = line.indexOf( ' ' );

            if ( o == -1 )
                continue;

            QString key = line.mid( 1, o - 1 );
            QString value = line.mid( o ).trimmed();

            if ( key == "IDENTIFIER" )
                m_identifier = value;
            else if ( key == "VERSION" )
                m_currentVersion = value.toInt();
            else if ( key == "LASTUPDATE" )
                m_lastUpdate = value.toLongLong();
        }
        else
        {
            // ID type file
            int o = line.indexOf( ' ' );

            if ( o == -1 && ( o = line.indexOf( '\t' )) == -1 )
                continue;

            Record rec;

            QString id = line.left( o );
            QString type = line.mid( o ).trimmed();

            if ( (o = type.indexOf( ' ' )) == -1 && ( o = line.indexOf( '\t' )) == -1 )
                continue;

            rec.type = type.left( o );
            rec.filename = type.mid( o + 1 ).trimmed();

            m_database[ id.toInt() ] = rec;
        }
    }

    return true;
}

bool SongDatabase::store()
{
    QFile fout( pSettings->songdbFilename );

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

QString SongDatabase::pathForId(int id) const
{
    QMap<int, Record>::const_iterator it = m_database.find( id );

    if ( it == m_database.end() )
        return "";

    return pSettings->songPathPrefix + QDir::separator() + it.value().filename;
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

bool SongDatabase::search(const QString &substr, QList<SongDatabase::SearchResult> &results, unsigned int limit)
{
    results.clear();

    for ( QMap<int, Record>::const_iterator it = m_database.begin(); it != m_database.end(); ++it )
    {
        const QString& path = it.value().filename;

        // Perform search on a full path first
        if ( path.indexOf( substr, 0, Qt::CaseInsensitive ) != -1 )
        {
            // Now split it and search again (as the first one may have succeed on a path component)
            SearchResult res;

            if ( !pathToArtistTitle( path, res.artist, res.title ) )
                continue;

            if ( res.artist.indexOf( substr, 0, Qt::CaseInsensitive ) == -1
                 && res.title.indexOf( substr, 0, Qt::CaseInsensitive ) == -1 )
                continue;

            res.id = it.key();
            results.append( res );

            if ( --limit == 0 )
                break;
        }
    }

    return !results.isEmpty();
}
