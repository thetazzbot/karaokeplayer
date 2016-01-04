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

#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>

#include "logger.h"
#include "settings.h"
#include "songqueue.h"
#include "karaokesong.h"
#include "actionhandler.h"
#include "convertermidi.h"
#include "songdatabase.h"

SongQueue * pSongQueue;

SongQueue::SongQueue(QObject *parent)
    : QObject( parent )
{
    m_currentSong = 0;
    m_nextQueueId = 1;
}

SongQueue::~SongQueue()
{
    if ( !pSettings->queueFilename.isEmpty() )
        QFile::remove( pSettings->queueFilename );
}

void SongQueue::init()
{
    if ( !pSettings->queueFilename.isEmpty() )
        load();
}

void SongQueue::addSong( Song song )
{
    // Find the currnet singer index
    int singeridx = 0;

    if ( !m_queue.isEmpty() && (int) m_currentSong < m_queue.size() )
        singeridx = qMax( m_singers.indexOf( m_queue[m_currentSong].singer ), 0 );

    if ( m_singers.indexOf( song.singer ) == -1 )
    {
        if ( pSettings->queueAddNewSingersNext )
            m_singers.insert( singeridx + 1, song.singer );
        else
            m_singers.append( song.singer );
    }

    // Add to the end now
    song.id = m_nextQueueId++;

    // Find out if it needs to be converted
    song.state = KaraokeSong::needsProcessing( song.file ) ? Song::STATE_PREPARING : Song::STATE_READY;

    // If it is, schedule it for conversion
    if ( song.state == Song::STATE_PREPARING )
        pConverterMIDI->add( song.file );

    // Add to queue
    m_queue.append( song );

    // Now we have to rearrange the songs from current to the end according to the singer order
    singeridx++;

    if ( singeridx >= m_singers.size() )
        singeridx = 0;

    int songidx = m_currentSong + 1;
    Logger::debug( "Queue: Adding file %d %s by %s into queue: %s", song.songid, qPrintable(song.file), qPrintable(song.singer), qPrintable( song.stateText() ) );
/*
    for ( int i = 0; i < m_singers.size(); i++ )
        qDebug() << "Singer " << i << ": " << m_singers[i];

    for ( int i = 0; i < m_queue.size(); i++ )
        qDebug() << "Queue " << i << ": " << m_queue[i].file << " " << m_queue[i].singer;
*/
    while ( songidx < m_queue.size() )
    {
        // This song should be sung by singeridx
//        qDebug("%d: should be next singer %s", songidx, qPrintable( m_singers[singeridx] ));

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

/*
    // Dump the queue
    qDebug() << "Queue after adding song" << file << "by" << singer;

    for ( int i = 0; i < m_queue.size(); i++ )
        qDebug() << "Queue " << i << ": " << m_queue[i].file << " " << m_queue[i].singer;
*/
    queueUpdated();
}

void SongQueue::addSong( const QString &singer, int id )
{
    SongDatabaseInfo info;

    if ( !pSongDatabase->songById( id, info ) )
        return;

    Song song;

    song.artist = info.artist;
    song.file = info.filePath;
    song.songid = id;
    song.singer = singer;
    song.title = info.title;

    addSong( song );
}

void SongQueue::addSong(const QString &singer, const QString &file)
{
    Song song;

    song.file = file;
    song.songid = 0;
    song.singer = singer;

    addSong( song );
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
    queueUpdated();
    return true;
}

bool SongQueue::next()
{
    if ( (int) m_currentSong >= m_queue.size() )
        return false;

    m_currentSong++;
    queueUpdated();
    return true;
}

void SongQueue::exportQueue(QList<SongQueue::Song> &queue)
{
    // Export starting from current song
    queue = m_queue.mid( m_currentSong );
}

bool SongQueue::isEmpty() const
{
    return (int) m_currentSong == m_queue.size() || m_queue.isEmpty();
}

void SongQueue::clear()
{
    m_queue.clear();
    m_currentSong = 0;

    queueUpdated();
}

void SongQueue::processingFinished(const QString &origfile, bool succeed)
{
    Logger::debug( "Processing of file %s finished %s", qPrintable(origfile), succeed ? "successfully" : "with errors" );

    // Find the file in queue - we go through the entire loop in case a file is added more than once
    for ( int i = 0; i < m_queue.size(); i++ )
    {
        if ( m_queue[i].file == origfile )
        {
            if ( succeed )
                m_queue[i].state = Song::STATE_READY;
            else
                m_queue.removeAt( i );
        }
    }

    emit queueUpdated();
}

void SongQueue::statusChanged(int id, bool playstarted)
{
    // Find the file in queue - we go through the entire loop in case a file is added more than once
    for ( int i = 0; i < m_queue.size(); i++ )
    {
        if ( m_queue[i].id == id )
        {
            m_queue[i].state = playstarted ? Song::STATE_PLAYING : Song::STATE_READY;
            break;
        }
    }

    emit queueUpdated();
}

void SongQueue::queueUpdated()
{
    if ( !pSettings->queueFilename.isEmpty() )
        save();

    emit queueChanged();
}

void SongQueue::save()
{
    QFile fout( pSettings->queueFilename );

    if ( !fout.open( QIODevice::WriteOnly ) )
    {
        pActionHandler->error( "Cannot store queue: " + fout.errorString() );
        return;
    }

    QDataStream dts( &fout );
    dts << QString("KARAOKEQUEUE");
    dts << (int) 1;
    dts << m_currentSong;
    dts << m_nextQueueId;
    dts << m_singers;
    dts << m_queue.size();

    foreach ( const Song& s, m_queue )
    {
        dts << s.id;
        dts << s.songid;
        dts << s.file;
        dts << s.title;
        dts << s.singer;
    }
}

void SongQueue::load()
{
    QFile fout( pSettings->queueFilename );

    if ( !fout.open( QIODevice::ReadOnly ) )
        return;

    QDataStream dts( &fout );
    QString header;
    int version;

    dts >> header;

    if ( header != "KARAOKEQUEUE" )
        return;

    dts >> version;

    if ( version != 1 )
        return;

    dts >> m_currentSong;
    dts >> m_nextQueueId;
    dts >> m_singers;
    dts >> version; // now queue size

    for ( int i = 0; i < version; i++ )
    {
        Song s;

        dts >> s.id;
        dts >> s.songid;
        dts >> s.file;
        dts >> s.title;
        dts >> s.singer;

        // Find out if it needs to be converted
        s.state = KaraokeSong::needsProcessing( s.file ) ? Song::STATE_PREPARING : Song::STATE_READY;

        // If it is, schedule it for conversion
        if ( s.state == Song::STATE_PREPARING )
            pConverterMIDI->add( s.file );

        m_queue.append( s );
    }

    if ( !m_queue.isEmpty() )
        emit queueChanged();
}
