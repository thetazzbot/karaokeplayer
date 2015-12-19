#ifndef SONGQUEUE_H
#define SONGQUEUE_H

#include <QObject>
#include <QList>
#include <QString>

// Represents the enqueued songs, maintains people's position
// and ensures honest rotation
class SongQueue : public QObject
{
    Q_OBJECT

    public:
        class Song
        {
            public:
                QString  file;
                QString  title;
                QString  singer;
        };

        SongQueue( QObject * parent );

        // Initializes the queue, loading it (and emitting the queueChanged) if necessary
        void    init();

        // Adds a song into queue, automatically rearranging it
        void    addSong( const QString& file, const QString& singer );

        // Returns true if the queue has no more songs and the current song is not valid
        bool    isEmpty() const;

        // Returns the current song from queue. Crashes if you call it and the queue is empty; use isEmpty first!
        Song    current() const;

        // Moves queue pointer to a previous song. Returns false if this is the first song (pointer doesn't move then)
        bool    prev();

        // Moves queue forward. Returns false if this is the last song (pointer doesn't move)
        bool    next();

        // Returns queue representation as list of author|song items (used in notifications
        void    asList( QList<QString>& queue );

        // Internal stuff
        QString filenameToTitle( const QString& file );

    signals:
        void    queueChanged();

    private:
        // Internal function which saves queue to disk and calls the signals
        void    queueUpdated();
        void    save();
        void    load();

        // Queue of songs. Works as following:
        //
        // - Songs are added into m_queue as they put in by people
        // - Songs are never removed; m_current points to the currently played song
        // - Current singer is set by m_currentSinger
        // - Once the song is done, the next one is chosen by the next singer; if none,
        //   then next singer, and so on (if there is song in the list, there is singer in m_singers)
        // - The current item is swapped with the singer (i.e. list is rearranged)
        // - List may go back into previous entries, then it is not rearranged. m_lastArranged controls this
        QList<Song>         m_queue;

        // The current song being played; if == m_queue.size(), nothing is played. Index in m_queue
        unsigned int        m_currentSong;

        // keeps track of all singers by name
        QList<QString>      m_singers;
};

extern SongQueue * pSongQueue;

#endif // SONGQUEUE_H
