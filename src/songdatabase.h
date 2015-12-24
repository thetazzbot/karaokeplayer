#ifndef SONGDATABASE_H
#define SONGDATABASE_H

#include <QMap>
#include <QObject>
#include <QStringList>

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

        // Queries the song file path
        QString pathForId( int id );

        // Returns the song lyric delay
        int     getLyricDelay( int id );

        // Updates the song playing stats and delay
        void    updatePlayedSong( int id, int newdelay );

    private:
        bool    pathToArtistTitle( const QString& path, QString& artist, QString& title );
        //bool    queryRow( const QString& sql, QMap<QString, QVariant> &output );
        bool    execute( const QString& sql, const QStringList& args = QStringList() );

    private:
        // Database intentifier (to recognize if the database changed)
        QString         m_identifier;

        // Current version of the database (to check for updates)
        int             m_currentVersion;

        // Last update time_t
        qint64          m_lastUpdate;

        // Database handle
        sqlite3 *       m_sqlitedb;
};

extern SongDatabase * pSongDatabase;

#endif // SONGDATABASE_H
