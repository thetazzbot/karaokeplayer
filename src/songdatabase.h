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

#ifndef SONGDATABASE_H
#define SONGDATABASE_H

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QStringList>

#include "songdatabasescanner.h"

struct sqlite3;

// Returned as result for the database query
class SongDatabaseInfo
{
    public:
        SongDatabaseInfo();

        int         id;
        QString     artist;
        QString     title;
        QString     filePath;
        QString     type;
        int         playedTimes;
        time_t      lastPlayed;
        int         lyricDelay;
        time_t      added;
        int         rating;
        QString     language;
};


// Initial design used SQLite, but since even the largest collections were below 2Mb in size,
// this made no sense. Only when we move to Android this might switch back to SQLite.
class SongDatabase : public QObject
{
    Q_OBJECT

    public:
        SongDatabase( QObject * parent );
        ~SongDatabase();

        // Initializes a new (empty) database, or loads an existing database
        bool    init();

        // Search for a substring in artists and titles
        bool    search( const QString& substr, QList<SongDatabaseInfo>& results, unsigned int limit = 100 );

        // Queries the song by ID
        bool    songById( int id, SongDatabaseInfo& info );

        // Queries the song by path
        bool    songByPath( const QString& path, SongDatabaseInfo& info );

        // Updates the song playing stats and delay
        void    updatePlayedSong( int id, int newdelay, int newrating );

        // For browsing the database
        bool    browseInitials( QList<QChar> &artistInitials );
        bool    browseArtists( const QChar& artistInitial, QStringList& artists );
        bool    browseSongs( const QString& artist, QList<SongDatabaseInfo>& results );

        // Language is stored in database as INT with three bytes encoding the country code in big-endian
        static QString languageFromInt( unsigned int value );

        // Add/update database entries from the list in a single transaction
        bool    updateDatabase( const QList<SongDatabaseScanner::SongDatabaseEntry> entries );
        bool    updateLastScan();

        // Empty the database
        bool    clearDatabase();

        // Get last update timestamp
        qint64  lastDatabaseUpdate() const;

        // Returns the song count
        unsigned int getSongCount() const;

    private:
        bool    verifyDatabaseVersion();
        bool    execute( const QString& sql, const QStringList& args = QStringList() );

    private:
        // Last update time
        qint64          m_lastUpdate;

        // Total number of songs in the database
        unsigned int    m_totalSongCount;

        // Database handle
        sqlite3 *       m_sqlitedb;
};

extern SongDatabase * pSongDatabase;

#endif // SONGDATABASE_H
