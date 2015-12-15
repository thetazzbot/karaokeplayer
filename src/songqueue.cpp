#include <QDebug>
#include <QFileInfo>

#include "songqueue.h"

SongQueue * pSongQueue;

SongQueue::SongQueue(QObject *parent)
    : QObject( parent )
{
    m_currentSong = 0;
/*
    addSong( "file1", "Vasya" );
    addSong( "file2", "Vasya" );
    addSong( "file3", "Petya" );
    addSong( "file4", "Igor" );
    addSong( "file5", "Petya" );
    addSong( "file6", "Petya" );
    addSong( "file7", "Petya" );
    addSong( "file8", "Igor" );
    m_currentSong = 1;
    addSong( "file9", "Vasya" );
    addSong( "file10", "John" );
    */
}

void SongQueue::addSong(const QString &file, const QString &singer)
{
    // Find the currnet singer index
    int singeridx = 0;

    if ( !m_queue.isEmpty() )
        singeridx = qMax( m_singers.indexOf( m_queue[m_currentSong].singer ), 0 );

    if ( m_singers.indexOf( singer ) == -1 )
        m_singers.insert( singeridx + 1, singer );

    // Add to the end now
    Song song;
    song.file = file;
    song.singer = singer;
    song.title = filenameToTitle( file );

    m_queue.append( song );

    // Now we have to rearrange the songs from current to the end according to the singer order
    singeridx++;

    if ( singeridx >= m_singers.size() )
        singeridx = 0;

    int songidx = m_currentSong + 1;
/*
    qDebug("\nAdding file %s by %s into queue, next singer %d", qPrintable(file), qPrintable(singer), singeridx );

    for ( int i = 0; i < m_singers.size(); i++ )
        qDebug() << "Singer " << i << ": " << m_singers[i];

    for ( int i = 0; i < m_queue.size(); i++ )
        qDebug() << "Queue " << i << ": " << m_queue[i].file << " " << m_queue[i].singer;
*/
    while ( songidx < m_queue.size() )
    {
        // This song should be sung by singeridx
        qDebug("%d: should be next singer %s", songidx, qPrintable( m_singers[singeridx] ));

        if ( m_queue[songidx].singer != m_singers[singeridx] )
        {
            // Find the song with this singer
            int i = songidx + 1;

            for ( ; i < m_queue.size(); i++ )
                if ( m_queue[i].singer == m_singers[singeridx] )
                    break;

            // Did we find a song with this singer?
            if ( i < m_queue.size() )
            {
                // Yes; swap with the current one, and move on
                m_queue.swap( songidx, i );
                songidx++;
            }
        }
        else
        {
            // This is already the case, so just skip this one
            songidx++;
        }

        // Switch to next singer
        singeridx++;

        if ( singeridx >= m_singers.size() )
            singeridx = 0;
    }

/*    // Dump the queue
    qDebug( ">>> Queue after sort:" );

    for ( int i = 0; i < m_queue.size(); i++ )
        qDebug() << "Queue " << i << ": " << m_queue[i].file << " " << m_queue[i].singer;
*/
    emit queueChanged();
}

SongQueue::Song SongQueue::current() const
{
    return m_queue[ m_currentSong ];
}

bool SongQueue::prev()
{
    if ( m_currentSong == 0  )
        return false;

    m_currentSong--;
    emit queueChanged();
    return true;
}

bool SongQueue::next()
{
    if ( (int) m_currentSong >= m_queue.size() )
        return false;

    m_currentSong++;
    emit queueChanged();
    return true;
}

void SongQueue::asList(QList<QString> &queue)
{
    queue.clear();

    for ( int i = m_currentSong + 1; i < m_queue.size(); i++ )
    {
        queue.append( m_queue[i].singer + "|" + m_queue[i].title );
    }
}

bool SongQueue::isEmpty() const
{
    return (int) m_currentSong == m_queue.size();
}

QString SongQueue::filenameToTitle(const QString &file)
{
    QFileInfo finfo( file );

    return finfo.baseName();
}
