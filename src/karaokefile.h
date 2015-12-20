#ifndef KARAOKEFILE_H
#define KARAOKEFILE_H

#include <QIODevice>
#include <QProcess>

#include "player.h"
#include "playerlyrics.h"
#include "playerbackground.h"

class Player;
class PlayerWidget;

// Represents a playable Karaoke file. It may be one of the following:
// - Two files (music + lyrics of proper format);
// - One archive file (such as ZIP) containing the above;
// - A compound file (such as KFN) containing the above;
//
// All preparation (and conversion) happens in the main thread, without a new thread
// being started. Then only when the player should be starting, the new thread
// starts too
class KaraokeFile : public QObject
{
    Q_OBJECT

    public:
        //KaraokeFile( AudioPlayer_FFmpeg * plr, PlayerWidget * w );
        KaraokeFile( PlayerWidget * w );

        ~KaraokeFile();

        // open - a music file, a lyrics file, a combined file (KFN) or a ZIP archive
        bool    open( const QString& filename );

        qint64  duration();
        qint64  position();

        qint64  draw( KaraokePainter& p );


    public slots:
        void    start();
        void    pause();
        void    seekForward();
        void    seekBackward();
        void    seekTo( qint64 timing );
        void    stop();

    private slots:
        void	convError( QProcess::ProcessError error );
        void	convFinished( int exitCode, QProcess::ExitStatus exitStatus );

    protected:
        enum State
        {
            STATE_RESET,
            STATE_READY,
            STATE_PLAYING,
            STATE_PAUSED
        };

        // Convert the src to the m_musicFileName
        void    startConversion( const QString& src );

        QString             m_musicFileName;
        PlayerLyrics     *  m_lyrics;
        PlayerBackground *  m_background;

        // For conversion
        QProcess         *  m_convProcess;

        // Player and rendering widget
        Player              m_player;
        PlayerWidget     *  m_widget;

        // Play state tracker
        State               m_playState;

        // Next redraw time
        qint64              m_lastRedrawTime;
        qint64              m_nextRedrawTime;

        static bool isMidiFile( const QString& filename );
        static bool isSupportedMusicFile( const QString& filename );
        static bool isSupportedLyricFile( const QString& filename );
};

#endif // KARAOKEFILE_H
