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
#include "languagedetector.h"
#include "util.h"


class SongDatabaseScannerWorkerThread : public QThread
{
    public:
        enum Type
        {
            THREAD_SCANNER,
            THREAD_PROCESSOR,
            THREAD_SUBMITTER
        };

        SongDatabaseScannerWorkerThread( SongDatabaseScanner * scanner, Type type ) : QThread( 0 )
        {
            m_type = type;
            m_scanner = scanner;
        }

        void run()
        {
            switch ( m_type )
            {
                case THREAD_SCANNER:
                    m_scanner->scanCollectionsThread();
                    break;

                case THREAD_PROCESSOR:
                    m_scanner->processingThread();
                    break;

                case THREAD_SUBMITTER:
                    m_scanner->submittingThread();
                    break;
            }
        }

    private:
        SongDatabaseScanner * m_scanner;
        Type                  m_type;
};



SongDatabaseScanner::SongDatabaseScanner(QObject *parent)
    : QObject(parent)
{
}

SongDatabaseScanner::~SongDatabaseScanner()
{
    if ( !m_threadPool.isEmpty() )
        stopScan();
}

bool SongDatabaseScanner::startScan()
{
    // Make a copy in case the settings change during scanning
    m_collection = pSettings->songCollection;

    // Do we need the language detector?
    bool need_lang_detector = false;

    for ( int i = 0; i < m_collection.size(); i++ )
        if ( m_collection[i].detectLanguage )
            need_lang_detector = true;

    if ( need_lang_detector )
    {
        m_langDetector = new LanguageDetector();

        if ( !m_langDetector->init() )
        {
            Logger::debug( "Failed to load the language detection library, language detection is not available: %s", qPrintable(m_langDetector->errorString()) );
            delete m_langDetector;
            m_langDetector = 0;
            return false;
        }
    }

    // Create the directory scanner thread
    m_threadPool.push_back( new SongDatabaseScannerWorkerThread( this, SongDatabaseScannerWorkerThread::THREAD_SCANNER ) );

    // Create the submitter thread
    m_threadPool.push_back( new SongDatabaseScannerWorkerThread( this, SongDatabaseScannerWorkerThread::THREAD_SUBMITTER ) );

    // And three processor threads
    for ( int i = 0; i < 3; i++ )
    {
        m_threadPool.push_back( new SongDatabaseScannerWorkerThread( this, SongDatabaseScannerWorkerThread::THREAD_PROCESSOR ) );
        m_threadsRunning++;
    }

    scanStarted = QDateTime::currentDateTime();
    m_abortScanning = 0;

    // And start all them
    Q_FOREACH( QThread * thread, m_threadPool )
        thread->start();

    return true;
}

void SongDatabaseScanner::stopScan()
{
    // Signal all running threads to stop
    m_abortScanning = 1;

    // Wait for all them and clean them up
    if ( !m_threadPool.isEmpty() )
    {
        Q_FOREACH( QThread * thread, m_threadPool )
        {
            thread->wait();
            delete thread;
        }
    }
}

void SongDatabaseScanner::scanCollectionsThread()
{
    Logger::debug( "SongDatabaseScanner: scanCollectionsThread started" );

    for ( int i = 0; i < m_collection.size(); i++ )
    {
        if ( m_abortScanning != 0 )
            break;

        // We do not use recursion, and use the queue-like list instead
        QStringList paths;
        paths << m_collection[i].rootPath;

        // And enumerate all the paths
        while ( !paths.isEmpty() )
        {
            QString current = paths.takeFirst();
            QFileInfo finfo( current );

            if ( m_abortScanning != 0 )
                break;

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
                    if ( fi.fileName().endsWith( ".zip", Qt::CaseInsensitive) && !m_collection[i].scanZips )
                        continue;

                    // Schedule for processing right away
                    SongDatabaseEntry entry;
                    entry.filePath = fi.absoluteFilePath();
                    entry.collection = i;
                    entry.language = m_collection[i].defaultLanguage;

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
                    entry.collection = i;
                    entry.language = m_collection[i].defaultLanguage;

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

    Logger::debug( "SongDatabaseScanner: scanCollectionsThread finished" );
}

void SongDatabaseScanner::processingThread()
{
    Logger::debug( "SongDatabaseScanner: procesing thread started" );

    while ( m_abortScanning == 0 )
    {
        // Wait until there are more entries in processing queue
        m_processingQueueMutex.lock();

        // Someone else might have taken our item
        if ( m_processingQueue.isEmpty() )
        {
            m_processingQueueCond.wait( &m_processingQueueMutex );

            // Since we're woken up here, our mutex is now locked
            if ( m_processingQueue.isEmpty() )
            {
                m_processingQueueMutex.unlock();
                continue;
            }
        }

        // An entry with empty path means no more entries (and we do not take it from queue to let other threads see it)
        if ( m_abortScanning || m_processingQueue.first().filePath.isEmpty() )
        {
            m_processingQueueMutex.unlock();
            break;
        }

        // Take our item and let others do work
        SongDatabaseEntry entry = m_processingQueue.takeFirst();
        m_processingQueueMutex.unlock();

        qDebug("Processing file %s", qPrintable(entry.filePath));
        karaokeFilesProcessed++;

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

        // Detecting the lyrics type
        int p = entry.filePath.lastIndexOf( '.' );

        if ( p == -1 )
            continue;

        entry.type = entry.filePath.mid( p + 1 ).toUpper();

        // Read the karaoke file
        QScopedPointer<KaraokePlayable> karaoke( KaraokePlayable::create( entry.filePath ) );

        if ( !karaoke || !karaoke->parse() )
        {
            Logger::debug( "SongDatabaseScanner: WARNING ignoring file %s as it cannot be parsed", qPrintable(entry.filePath) );
            continue;
        }

        // For non-CDG files we can do the artist/title and language detection from source
        if ( !karaoke->lyricObject().endsWith( ".cdg", Qt::CaseInsensitive ) && m_collection[entry.collection].detectLanguage && m_langDetector )
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
            entry.language = m_langDetector->detectLanguage( lyrics->exportAsText().toUtf8() );

            // Fill up the artist/title if we detected them
            if ( lyrics->properties().contains( LyricsLoader::PROP_ARTIST ) )
                entry.artist = lyrics->properties()[ LyricsLoader::PROP_ARTIST ];

            if ( lyrics->properties().contains( LyricsLoader::PROP_TITLE ) )
                entry.title = lyrics->properties()[ LyricsLoader::PROP_TITLE ];

            if ( lyrics->properties().contains( LyricsLoader::PROP_LYRIC_SOURCE ) )
                entry.type += "/" + lyrics->properties()[ LyricsLoader::PROP_LYRIC_SOURCE ];
        }

        // Get the artist/title from path according to settings
        if ( !guessArtistandTitle( entry ) )
        {
            Logger::debug( "SongDatabaseScanner: failed to detect artist/title for file %s, skipped", qPrintable(entry.filePath) );
            continue;
        }

        Logger::debug( "SongDatabaseScanner: added/updated song %s by %s, type %s, language %s, file %s",
                       qPrintable(entry.title),
                       qPrintable(entry.artist),
                       qPrintable(entry.type),
                       qPrintable( LanguageDetector::languageFromCode( entry.language ) ),
                       qPrintable(entry.filePath) );

        addSubmitting( entry );

    }

    // If m_threadsRunning was 1 when this thread finished, this is the last one before the submitting thread
    if ( m_threadsRunning.fetchAndAddAcquire( -1 ) == 1 )
    {
        // Signal that no more data would be submitted - shut down
        Logger::debug( "SongDatabaseScanner: last procesing thread finished, signaling the submitter to shut down" );
        m_abortScanning = 1;

        // In case the submitter is waiting in condition
        m_submittingQueueCond.wakeAll();
    }
    else
        Logger::debug( "SongDatabaseScanner: procesing thread finished" );
}

void SongDatabaseScanner::submittingThread()
{
    const int ENTRIES_TO_UPDATE = 500;

    Logger::debug( "SongDatabaseScanner: submitting thread started" );

    while ( m_abortScanning == 0 )
    {
        // Wait until there are more entries in processing queue
        m_submittingQueueMutex.lock();

        // Someone else might have taken our item
        if ( m_submittingQueue.size() >= ENTRIES_TO_UPDATE  )
        {
            pSongDatabase->updateDatabase( m_submittingQueue );
            m_submittingQueue.clear();
        }

        m_submittingQueueCond.wait( &m_submittingQueueMutex );
        m_submittingQueueMutex.unlock();
    }

    // Submit the rest, if any
    if ( !m_submittingQueue.isEmpty() )
        pSongDatabase->updateDatabase( m_submittingQueue );

    Logger::debug( "SongDatabaseScanner: submitter thread finished, scan completed" );
    emit finished();
}

void SongDatabaseScanner::addProcessing(const SongDatabaseScanner::SongDatabaseEntry &entry)
{
    karaokeFilesFound++;
    m_processingQueueMutex.lock();
    m_processingQueue.push_back( entry );
    m_processingQueueMutex.unlock();
    m_processingQueueCond.wakeOne();
}

void SongDatabaseScanner::addSubmitting(const SongDatabaseScanner::SongDatabaseEntry &entry)
{
    m_submittingQueueMutex.lock();
    m_submittingQueue.push_back( entry );
    m_submittingQueueMutex.unlock();
    m_submittingQueueCond.wakeOne();
}

bool SongDatabaseScanner::guessArtistandTitle( SongDatabaseScanner::SongDatabaseEntry &entry )
{
    if ( m_collection[entry.collection].pathFormat == Settings::Collection::PATH_FORMAT_ARTIST_DASH_TITLE )
    {
        int p = entry.filePath.lastIndexOf( QDir::separator() );

        if ( p != -1 )
            entry.artist = entry.filePath.mid( p + 1 ); // has path component
        else
            entry.artist = entry.filePath; // no path component

        p = entry.artist.indexOf( " - " );

        if ( p == -1 )
            return false;

        entry.title = entry.artist.mid( p + 3 );
        entry.artist = entry.artist.left( p );
    }
    else
    {
        int p = entry.filePath.lastIndexOf( QDir::separator() );

        if ( p == -1 )
            return false;

        entry.title = entry.filePath.mid( p + 1 );
        entry.artist = entry.filePath.left( p );

        // Trim down the artist if needed - may not be there (i.e. "Adriano Celentano/Le tempo.mp3")
        if ( (p = entry.artist.lastIndexOf( QDir::separator() )) != -1 )
            entry.artist = entry.artist.mid( p + 1 );

        // Trim the extension from title
        if ( (p = entry.title.lastIndexOf( '.' )) != -1 )
            entry.title = entry.title.left( p );
    }

    return true;
}
