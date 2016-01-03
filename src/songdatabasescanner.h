#ifndef SONGDATABASESCANNER_H
#define SONGDATABASESCANNER_H

#include <QQueue>
#include <QObject>
#include <QAtomicInt>
#include <QWaitCondition>
#include <QMutex>
#include <QDateTime>

#include "settings.h"


class SongDatabaseScannerWorkerThread;
class LanguageDetector;

class SongDatabaseScanner : public QObject
{
    Q_OBJECT

    public:
        explicit SongDatabaseScanner( QObject *parent = 0 );
        ~SongDatabaseScanner();

        class SongDatabaseEntry
        {
            public:
                int         collection; // to find out the scan options
                QString     artist;
                QString     title;
                QString     filePath;   // main file - the compound file or lyrics
                QString     musicPath;  // if music file is separate, will be used to get artist/title if not available otherwise
                QString     type;       // karaoke format such as cdg, avi, or midi; could also be extended such as lrc/minus
                int         language;   // the language value if unknown/impossible to detect
        };

        // Scan runtime parameters
        QDateTime           scanStarted;
        QAtomicInt          karaokeFilesFound;
        QAtomicInt          karaokeFilesProcessed;

    signals:
        // Scan finished
        void    finished();

    public slots:
        bool    startScan();
        void    stopScan();


    private:
        friend class SongDatabaseScannerWorkerThread;

        // This thread scans directories and finds out karaoke files. It only performs directory enumeration,
        // and does not read any files nor accesses the database.
        void    scanCollectionsThread();

        // This thread (probably a few) reads the lyric files (if they're text files) and gets the artist/title
        // and language information if available. This is not possible for all formats (not for CD+G for example).
        // Those threads access database for reading only.
        void    processingThread();

        // This thread submits the new entries into the database.
        void    submittingThread();

        // Find out the artist and title from lyrics, music or file path.
        bool    guessArtistandTitle( SongDatabaseEntry& entry );


        // Producer-consumer implementation of processing queue
        QMutex                      m_processingQueueMutex;
        QWaitCondition              m_processingQueueCond;
        QQueue<SongDatabaseEntry>   m_processingQueue;

        // Adding an entry into the processing queue
        void    addProcessing( const SongDatabaseEntry& entry );

        // Producer-consumer implementation of database submitting queue
        QMutex                      m_submittingQueueMutex;
        QWaitCondition              m_submittingQueueCond;
        QList<SongDatabaseEntry>    m_submittingQueue;

        // Adding an entry into the submitting queue
        void    addSubmitting( const SongDatabaseEntry& entry );

        QAtomicInt                  m_abortScanning;

        // Copy of collection for scanning
        QList<Settings::Collection> m_collection;

        // A module (auto-loaded) to detect the lyric language
        LanguageDetector    *       m_langDetector;

        // Thread pool of scanners
        QList< QThread * >          m_threadPool;

        // Number of threads completing the task (to send finished)
        QAtomicInt                  m_threadsRunning;
};

#endif // SONGDATABASESCANNER_H
