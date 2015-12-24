#ifndef SONGDATABASE_H
#define SONGDATABASE_H

#include <QMap>
#include <QObject>

struct sqlite3;

// Initial design used SQLite, but since even the largest collections were below 2Mb in size,
// this made no sense. Only when we move to Android this might switch back to SQLite.
class SongDatabase : public QObject
{
    Q_OBJECT

    public:
        class SearchResult
        {
        public:
            int         id;
            QString     artist;
            QString     title;
        };

        SongDatabase( QObject * parent );
        ~SongDatabase();

        bool init();

        // Initializes a new (empty) database, or loads an existing database
        bool    importFromText(const QString& filename , const QString &pathPrefix);
        //bool    exportToText( const QString& filename );

        bool    search( const QString& substr, QList<SearchResult>& results, unsigned int limit = 100 );

        QString pathForId( int id );

    private:
        bool    pathToArtistTitle( const QString& path, QString& artist, QString& title );
        bool    queryRow( const QString& sql, QMap<QString, QVariant> &output );
        bool    execute( const QString& sql );
        QString &quote( QString& orig );
        QString quote( const QString& orig );

    private:
        class Record
        {
        public:
            QString type;
            QString filename;
        };

        // Database intentifier (to recognize if the database changed)
        QString         m_identifier;

        // Current version of the database (to check for updates)
        int             m_currentVersion;

        // Last update time_t
        qint64          m_lastUpdate;

        // Prefix for the file paths
        QString         m_filePrefix;

        // Database handle
        sqlite3 *       m_sqlitedb;
};

extern SongDatabase * pSongDatabase;

#endif // SONGDATABASE_H
