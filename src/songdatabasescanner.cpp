#include <QDir>
#include <QMap>
#include <QFileInfo>
#include <QThread>
#include <QDateTime>

#include "logger.h"
#include "karaokeplayable.h"
#include "songdatabasescanner.h"
#include "songdatabase.h"
#include "playerlyricstext.h"
#include "settings.h"
#include "languagedetector.h"
#include "util.h"


class DirectoryScanThread : public QThread
{
    public:
        DirectoryScanThread( SongDatabaseScanner * scanner, QObject * obj = 0 ) : QThread( obj )
        {
            m_scanner = scanner;
        }

        void run()
        {
            m_scanner->scanCollectionsThread();
            m_scanner->processingThread();
        }

    private: SongDatabaseScanner * m_scanner;
};

SongDatabaseScanner::SongDatabaseScanner(QObject *parent)
    : QObject(parent)
{
    m_threadScanDirs = 0;
}

void SongDatabaseScanner::startScan()
{
    m_langDetector = new LanguageDetector();

    if ( !m_langDetector->init() )
    {
        Logger::debug( "Failed to load the language detection library, language detection is not available: %s", qPrintable(m_langDetector->errorString()) );
        delete m_langDetector;
        m_langDetector = 0;
    }

    m_threadScanDirs = new DirectoryScanThread( this );
    m_threadScanDirs->start();
}

void SongDatabaseScanner::stopScan()
{

}

void SongDatabaseScanner::scanCollectionsThread()
{
    // Make a copy in case the settings change during scanning
    QList<Settings::Collection> collection = pSettings->songCollection;

    Q_FOREACH ( Settings::Collection col, collection )
    {
        // We do not use recursion, and use the queue-like list instead
        QStringList paths;
        paths << col.rootPath;

        // And enumerate all the paths
        while ( !paths.isEmpty() )
        {
            QString current = paths.takeFirst();

            QFileInfo finfo( current );

            // Skip non-existing paths (might come from settings)
            if ( !finfo.exists() )
                continue;

            // We assume the situation where we have several lyrics for a single music is more prevalent
            QMap< QString, QString > musicFiles, lyricFiles;

            Q_FOREACH( QFileInfo fi, QDir( current ).entryInfoList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::DirsFirst ) )
            {
                // Re-add directories into the list
                if ( fi.isDir() )
                {
                    paths.push_back( fi.absoluteFilePath() );
                    continue;
                }

                // No more directories in this dir, but we have files
                if ( KaraokePlayable::isSupportedCompleteFile( fi.fileName()) )
                {
                    // We only add zip files if enabled for this collection
                    if ( fi.fileName().endsWith( ".zip", Qt::CaseInsensitive) && !col.scanZips )
                        continue;

                    // Schedule for processing right away
                    SongDatabaseEntry entry;
                    entry.filePath = fi.absoluteFilePath();
                    entry.language = col.defaultLanguage;
                    entry.detectLang = col.detectLanguage;

                    addProcessing( entry );
                }
                else if ( KaraokePlayable::isSupportedMusicFile( fi.fileName()) )
                {
                    // Music is mapped basename -> file
                    musicFiles[ fi.baseName() ] = fi.fileName();
                }
                else if ( KaraokePlayable::isSupportedLyricFile( fi.fileName()) )
                {
                    // But lyric is mapped file -> basename
                    lyricFiles[ fi.fileName() ] = fi.baseName();
                }
                else
                    Logger::debug( "SongDatabaseScanner: unknown file %s, skipping", qPrintable( fi.absoluteFilePath() ) );
            }

            // Try to see if we have all matched music-lyric files - and move them to completeFiles
            while ( !lyricFiles.isEmpty() )
            {
                QString lyric = lyricFiles.firstKey();
                QString lyricbase = lyricFiles.take( lyric );

                if ( musicFiles.contains( lyricbase ) )
                {
                    // And we have a complete song
                    SongDatabaseEntry entry;
                    entry.filePath = current + QDir::separator() + lyric;
                    entry.musicPath = musicFiles[ lyricbase ];
                    entry.language = col.defaultLanguage;
                    entry.detectLang = col.detectLanguage;

                    addProcessing( entry );
                }
                else
                    Logger::debug( "SongDatabaseScanner: WARNING no music found for lyric file %s", qPrintable( lyric) );
            }
/*
        // Now see if there are music files left without lyrics
        for ( QMap< QString, QString >::iterator it = musicFiles.begin(); it != musicFiles.end(); ++it )
            if ( !it.value().isEmpty() )
                Logger::debug( "SongDatabaseScanner: WARNING no lyrics found for music file %s", qPrintable( it.key() ) );
*/
        }
    }

    // All done - put an entry with an empty path
    addProcessing( SongDatabaseEntry() );
}

void SongDatabaseScanner::processingThread()
{
    while ( m_abortScanning == 0 )
    {
        // Wait until there are more entries in processing queue
        m_processingQueueMutex.lock();

        // Someone else might have taken our item
        if ( m_processingQueue.isEmpty() )
        {
            m_processingQueueCond.wait( &m_processingQueueMutex );
            continue;
        }

        // An entry with empty path means no more entries (and we do not take it from queue to let other threads see it)
        if ( m_abortScanning || m_processingQueue.first().filePath.isEmpty() )
            break;

        // Take our item and let others do work
        SongDatabaseEntry entry = m_processingQueue.takeFirst();
        m_processingQueueMutex.unlock();

        qDebug("Processing file %s", qPrintable(entry.filePath));

        // Query the database to see what, if anything we already have for this path
        SongDatabaseInfo info;

        if ( pSongDatabase->songByPath( entry.filePath, info ) )
        {
            // We have the song, does it have all the information?
            if ( !info.artist.isEmpty() && info.title.isEmpty() && !info.type.isEmpty() && info.language != 0 )
            {
                // Is it up-to-date?
                if ( QFileInfo(entry.filePath).lastModified() <= QDateTime::fromMSecsSinceEpoch( info.added * 1000LL ) )
                    continue;
            }
        }

        // Read the karaoke file
        QScopedPointer<KaraokePlayable> karaoke( KaraokePlayable::create( entry.filePath ) );

        if ( !karaoke || !karaoke->parse() )
        {
            Logger::debug( "SongDatabaseScanner: WARNING ignoring file %s as it cannot be parsed", qPrintable(entry.filePath) );
            continue;
        }

        // For non-CDG files we can do the artist/title and language detection from source
        if ( !karaoke->lyricObject().endsWith( ".cdg", Qt::CaseInsensitive ) && entry.detectLang )
        {
            // Get the pointer to the device the lyrics are stored in (will be deleted after the pointer out-of-scoped)
            QScopedPointer<QIODevice> lyricDevice( karaoke->openObject( karaoke->lyricObject() ) );

            if ( lyricDevice == 0 )
            {
                Logger::debug( "SongDatabaseScanner: WARNING cannot open lyric file %s in karaoke file %s", qPrintable( karaoke->lyricObject() ), qPrintable(entry.filePath) );
                continue;
            }

            QScopedPointer<PlayerLyricsText> lyrics( new PlayerLyricsText( "", "", &Util::detectEncoding ) );

            if ( !lyrics->load( lyricDevice.data(), karaoke->lyricObject() ) )
            {
                if ( karaoke->lyricObject() != entry.filePath )
                    Logger::debug( "SongDatabaseScanner: karaoke file %s contains invalid lyrics %s", qPrintable(entry.filePath), qPrintable( karaoke->lyricObject() ) );
                else
                    Logger::debug( "SongDatabaseScanner: lyrics file %s cannot be loaded", qPrintable(entry.filePath) );

                continue;
            }

            // Detect the language
            int lang = m_langDetector->detectLanguage( lyrics->exportAsText().toUtf8() );
            qDebug("lang detected: %d (%s)", lang, qPrintable(m_langDetector->languageFromCode( lang ) ) );
        }
    }
}

void SongDatabaseScanner::addProcessing(const SongDatabaseScanner::SongDatabaseEntry &entry)
{
    m_processingQueueMutex.lock();
    m_processingQueue.push_back( entry );
    m_processingQueueMutex.unlock();
    m_processingQueueCond.wakeOne();
}
