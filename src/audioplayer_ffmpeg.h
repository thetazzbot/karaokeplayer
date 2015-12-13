/**************************************************************************
 *  Karlyriceditor - a lyrics editor and CD+G / video export for Karaoke  *
 *  songs.                                                                *
 *  Copyright (C) 2009-2013 George Yunaev, support@ulduzsoft.com          *
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

#ifndef AUDIOPLAYER_FFMPEG_H
#define AUDIOPLAYER_FFMPEG_H

#include <QMutex>
#include <QString>
#include <QAudioOutput>

#include "ffmpeg_headers.h"


class AudioBuffer;


// Audio player based on FFMpeg(tm) decoders and QAudioOutput
class AudioPlayer_FFmpeg : public QObject
{
	Q_OBJECT

	public:
        AudioPlayer_FFmpeg();
        virtual ~AudioPlayer_FFmpeg();

		// If a function below returned an error, this function will
		// retrieve the error message
		QString	errorMsg() const;

		// Initialize the player
		bool	init();

		// Open the audio file. Returns true on success, false otherwise
        bool	load( const QString& filename );

		// Closes the audio file. If another file is opened, the previous will
		// be closed automatically.
        //void	close();

		// True if audio is playing
		bool	isPlaying() const;

        // Current playing position
        qint64	position() const;

        // The audio file duration
        qint64	duration() const;

		// Metadata
		QString	metaTitle() const;
		QString	metaArtist() const;

	public slots:
		// Start or continues playing
        void	start();

		// Pauses the playing
		void	stop();

		// Rewinds the music file back; to emulate real "stop" call "stop" and "reset"
        void	reset();

		// Rewinds to a specific position
        //void	seekTo( qint64 value );

	private:
        // Meta tags
        QString         m_metaTitle;
        QString         m_metaArtist;
        QString         m_metaAlbum;

    private:
        // Called from the callback
        bool    MoreAudio();
        void    queueClear();

    private:
        QString         m_errorMsg;
        bool            m_audioOpened;

        // Access to everything below is guarded by mutex
        mutable QMutex  m_mutex;

        QAudioOutput *  m_output;
        AudioBuffer *   m_sampleBuffer;
};


#endif // AUDIOPLAYERPRIVATE_H
