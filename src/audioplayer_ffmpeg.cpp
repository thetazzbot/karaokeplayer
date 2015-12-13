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

#include <QMessageBox>
#include <QAudioFormat>
#include <QAtomicInteger>

#include "audioplayer_ffmpeg.h"

class AudioBuffer : public QIODevice
{
    public:
        AudioBuffer()
        {
            pFormatCtx = 0;
            aCodecCtx = 0;
            pCodec = 0;

            m_playing = 0;
            m_position = 0;

            clear();
        }

        virtual ~AudioBuffer()
        {
        }

        bool open( const QString& filename )
        {
            // Open the file
            if ( avformat_open_input( &pFormatCtx, FFMPEG_FILENAME( filename ), NULL, 0 ) != 0 )
            {
                setErrorString( "Could not open the audio file" );
                return false;
            }

            // Retrieve stream information
            if ( avformat_find_stream_info( pFormatCtx, 0 ) < 0 )
            {
                setErrorString( "Could not find stream information in the audio file" );
                return false;
            }

            // Find the first audio stream
            audioStream = -1;

            for ( unsigned i = 0; i < pFormatCtx->nb_streams; i++ )
            {
                if ( pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO )
                {
                    audioStream = i;
                    break;
                }
            }

            if ( audioStream == -1 )
            {
                setErrorString( "This file does not contain any playable audio" );
                return false;
            }

            if ( pFormatCtx->streams[audioStream]->duration == (int64_t) AV_NOPTS_VALUE )
            {
                setErrorString( "Cannot determine the total audio length" );
                return false;
            }
/*
            // Extract some metadata
            AVDictionary* metadata = pFormatCtx->metadata;

            if ( metadata )
            {
                m_metaTitle = getMetaTag( metadata, "title" );
                m_metaArtist = getMetaTag( metadata, "artist" );
            }
*/
            m_duration = av_rescale_q( pFormatCtx->streams[audioStream]->duration,
                                       pFormatCtx->streams[audioStream]->time_base,
                                       AV_TIME_BASE_Q ) / 1000;

            aCodecCtx = pFormatCtx->streams[audioStream]->codec;
            aCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_S16;

            // Open audio codec
            AVCodec * aCodec = avcodec_find_decoder( aCodecCtx->codec_id );

            if ( !aCodec || avcodec_open2( aCodecCtx, aCodec, 0 ) != 0 )
            {
                setErrorString( "Unsupported audio codec" );
                return false;
            }

            // Allocate the buffer
            m_frame = avcodec_alloc_frame();

            if ( !m_frame )
            {
                setErrorString( "Cannot allocate frame memory buffer" );
                return false;
            }

            // Init the packet queue
            clear();

            QIODevice::open( QIODevice::ReadOnly );
            return true;
        }

        virtual qint64 readData(char * data, qint64 maxSize)
        {
            int dataoffset = 0;

            while ( dataoffset < maxSize )
            {
                if ( m_offset >= m_buffer.size() )
                {
                    // We have already sent all our data; get more
                    if ( !readMoreAudio() )
                    {
                        if ( dataoffset > 0 )
                        return dataoffset;

                        return -1;
                    }
                }

                int len = qMin( maxSize - dataoffset, (qint64) m_buffer.size() - m_offset );
                memcpy( data + dataoffset, (uint8_t *) m_buffer.data() + m_offset, len );
                m_offset += len;
                dataoffset += len;
            }

            return dataoffset;
        }

        virtual qint64 writeData(const char * data, qint64 maxSize)
        {
            return -1;
        }

        void clear()
        {
            m_offset = 0;
            m_buffer.clear();
        }

        bool readMoreAudio()
        {
            while ( m_playing )
            {
                AVPacket packet;

                // Read a frame
                if ( av_read_frame( pFormatCtx, &packet ) < 0 )
                    return false;  // Frame read failed (e.g. end of stream)

                if ( packet.stream_index != audioStream )
                {
                    av_free_packet( &packet );
                    continue;
                }

                m_offset = 0;
                m_buffer.clear();

                m_position = av_rescale_q( packet.pts,
                                           pFormatCtx->streams[audioStream]->time_base,
                                           AV_TIME_BASE_Q ) / 1000;

                // Save the orig data so we can call av_free_packet() on it
                void * porigdata = packet.data;

                while ( packet.size > 0 )
                {
                    int got_frame_ptr;
                    int len = avcodec_decode_audio4( aCodecCtx, m_frame, &got_frame_ptr, &packet );

                    if ( len < 0 )
                    {
                        // if error, skip frame
                        break;
                    }

                    packet.data += len;
                    packet.size -= len;

                    if ( !got_frame_ptr )
                        continue;

                    void * samples = m_frame->data[0];
                    int decoded_data_size = av_samples_get_buffer_size( NULL,
                                                                        aCodecCtx->channels,
                                                                        m_frame->nb_samples,
                                                                        aCodecCtx->sample_fmt, 1 );

                    int cur = m_buffer.size();
                    int newsize = cur + decoded_data_size;
                    m_buffer.resize( newsize );
                    memcpy( m_buffer.data() + cur, samples, decoded_data_size );
                }

                packet.data = (uint8_t*) porigdata;
                av_free_packet( &packet );
                return true;
            }

            return false;
        }

    public:
        QByteArray           m_buffer;
        unsigned int         m_offset;

        AVFormatContext    * pFormatCtx;
        int                  audioStream;
        AVCodecContext     * aCodecCtx;
        AVCodec            * pCodec;
        AVFrame            * m_frame;

        qint64              m_duration;

        QAtomicInteger<qint64>  m_position;
        QAtomicInt          m_playing;
};




AudioPlayer_FFmpeg::AudioPlayer_FFmpeg()
{
    m_sampleBuffer = new AudioBuffer();
}

AudioPlayer_FFmpeg::~AudioPlayer_FFmpeg()
{
    delete m_sampleBuffer;
}

bool AudioPlayer_FFmpeg::isPlaying() const
{
    QMutexLocker m( &m_mutex );
    return m_sampleBuffer->m_playing;
}

qint64 AudioPlayer_FFmpeg::position() const
{
    QMutexLocker m( &m_mutex );
    return m_sampleBuffer->m_position;
}

qint64 AudioPlayer_FFmpeg::duration() const
{
    QMutexLocker m( &m_mutex );
    return m_sampleBuffer->m_duration;
}

QString AudioPlayer_FFmpeg::errorMsg() const
{
    return m_errorMsg;
}
/*
void AudioPlayer_FFmpeg::close()
{
    QMutexLocker m( &m_mutex );
    m_output->stop();

    // Close the codec
    if ( aCodecCtx )
        avcodec_close( aCodecCtx );

    // Close the video file
    if ( pFormatCtx )
        avformat_close_input( &pFormatCtx );

    if ( m_output )
    {
        delete m_output;
        m_output = 0;
        m_audioOpened = false;
    }

    if ( m_frame )
        av_free( m_frame );

    m_frame = 0;
    pFormatCtx = 0;
    aCodecCtx = 0;
    pCodec = 0;
}
*/
/*
static QString getMetaTag( AVDictionary* meta, const char * tagname )
{
    AVDictionaryEntry * ent = av_dict_get(meta, tagname, NULL, 0);

    if ( ent )
        return QString::fromUtf8( ent->value );
    else
        return "";
}
*/
bool AudioPlayer_FFmpeg::load( const QString& filename )
{
    // Init FFMpeg stuff
    ffmpeg_init_once();

    // Close if opened
    //close();

    if ( !m_sampleBuffer->open( filename ) )
    {
        m_errorMsg = m_sampleBuffer->errorString();
        return false;
    }

    // Now initialize the audio device
    QAudioFormat format;

    format.setCodec("audio/pcm");
    format.setSampleSize( 16 );
    format.setSampleRate( m_sampleBuffer->aCodecCtx->sample_rate );
    format.setChannelCount( m_sampleBuffer->aCodecCtx->channels );
    format.setByteOrder( QAudioFormat::LittleEndian );
    format.setSampleType( QAudioFormat::SignedInt );

    QAudioDeviceInfo info( QAudioDeviceInfo::defaultOutputDevice() );

    if ( !info.isFormatSupported(format) )
    {
        m_errorMsg = "Audio device doesn't support wav output";
        return false;
    }

    m_output = new QAudioOutput( format, this );
    return true;
}


void AudioPlayer_FFmpeg::start()
{
    QMutexLocker m( &m_mutex );
    m_sampleBuffer->m_playing.store( 1 );

    m_output->start( m_sampleBuffer );
}

void AudioPlayer_FFmpeg::reset()
{
//    seekTo( 0 );
}

void AudioPlayer_FFmpeg::stop()
{
    QMutexLocker m( &m_mutex );
    m_sampleBuffer->m_playing.store( 0 );
    m_output->stop();
}
/*
void AudioPlayer_FFmpeg::seekTo( qint64 value )
{
    QMutexLocker m( &m_mutex );

    av_seek_frame( pFormatCtx, -1, value * 1000, 0 );
    avcodec_flush_buffers( aCodecCtx );

    m_output->clear();
}
*/
