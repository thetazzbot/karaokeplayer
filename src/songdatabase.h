#ifndef SONGDATABASE_H
#define SONGDATABASE_H

#include <QMap>
#include <QObject>

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

        SongDatabase();

        // Initializes a new (empty) database, or loads an existing database
        bool    init();
        bool    store();

        bool    search( const QString& substr, QList<SearchResult>& results, unsigned int limit = 100 );

        QString pathForId( int id ) const;

    private:
        bool    pathToArtistTitle( const QString& path, QString& artist, QString& title );

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

        // Database itself
        QMap<int, Record>  m_database;
};

extern SongDatabase * pSongDatabase;

#endif // SONGDATABASE_H
