#ifndef SONGDATABASESCANNER_H
#define SONGDATABASESCANNER_H

#include <QQueue>
#include <QObject>
#include <QAtomicInt>
#include <QWaitCondition>
#include <QMutex>

class DirectoryScanThread;

class SongDatabaseScanner : public QObject
{
    Q_OBJECT

    public:
        explicit SongDatabaseScanner( QObject *parent = 0 );

    signals:

    public slots:
        void    startScan();
        void    stopScan();

    private:
        friend class DirectoryScanThread;

        class SongDatabaseEntry
        {
            public:
                QString     artist;
                QString     title;
                QString     filePath;   // main file - the compound file or lyrics
                QString     musicPath;  // if music file is separate, will be used to get artist/title if not available otherwise
                QString     type;       // karaoke format such as cdg, avi, or midi; could also be extended such as lrc/minus
                int         language;   // the language value if unknown/impossible to detect
                bool        detectLang; // if false, we do not attempt to detect language
        };

        // This thread scans directories and finds out karaoke files. It only performs directory enumeration,
        // and does not read any files nor accesses the database.
        void    scanCollectionsThread();

        // This thread (probably a few) reads the lyric files (if they're text files) and gets the artist/title
        // and language information if available. This is not possible for all formats (not for CD+G for example).
        // Those threads access database for reading only.
        void    processingThread();

        // Adding an entry into the processing queue
        void    addProcessing( const SongDatabaseEntry& entry );

        // Producer-consumer implementation of processing queue
        QMutex                      m_processingQueueMutex;
        QWaitCondition              m_processingQueueCond;
        QQueue<SongDatabaseEntry>   m_processingQueue;

        QAtomicInt                  m_abortScanning;

        DirectoryScanThread *       m_threadScanDirs;
};

#endif // SONGDATABASESCANNER_H
