#ifndef UNIVERSALPLAYER_H
#define UNIVERSALPLAYER_H

#include <QMutex>
#include <QObject>
#include <QString>
#include <QAtomicInt>
#include <QMediaPlayer>

class KaraokePainter;
class QMediaPlayer;
class UniversalPlayerVideoSurface;


// A universal player class - can play videos or music. Must support multiple instances
// (typically one plays video, and one plays music)
//
class UniversalPlayer : public QObject
{
    Q_OBJECT

    public:
        explicit UniversalPlayer(QObject *parent = 0);
        ~UniversalPlayer();

        // Load the audio file, no video output is possible. Returns true on success (ready to start),
        // false on failure (see errorMsg() then)
        bool    loadAudio( const QString& musicfile );

        // Load the video file, video output is possible. Returns true on success (ready to start),
        // false on failure (see errorMsg() then)
        bool    loadVideo( const QString& videofile, bool muted );

        // Draws the video content (only for video player) on the image. For non-video does nothing.
        // This function is called from a different thread, so locking is necessary.
        void    draw( KaraokePainter& p );

    public slots:
        // Start playing (may be delayed until the media is loaded)
        void    play();

        // Stops playing; for video the rendering image is not updated anymore
        // (but still remains valid)
        void    stop();

        // Pause or resume
        void    pauseOrResume( bool pause );

        // Seek to position
        void    seekTo( qint64 time );

        // Current position
        qint64  position();

        // Decrease or increase volume
        void    volumeDown();
        void    volumeUp();

        // Media duration, if known
        qint64  duration();

        // Error message, if any
        QString errorMsg() const { return m_errorMsg; }

    signals:
        // Media is finished
        void    finished();

    private slots:
        void	slotError( QMediaPlayer::Error error );
        void    slotMediaStatusChanged(QMediaPlayer::MediaStatus status);

    private:
        bool    waitForMediaLoaded();
        void    newMediaPlayer( bool video, bool muted );

        QString         m_errorMsg;

        // The actual player
        QMediaPlayer  * m_player;

        // If video playing is enabled, it draws on this surface
        UniversalPlayerVideoSurface *  m_surface;

        // Tracks loading of the media
        QAtomicInt      m_mediaLoadedOrFailed;

        // Tracks playing status (for draw() calls) - mutex prevents destroying the player while drawing
        QAtomicInt      m_drawEnabled;
        QMutex          m_drawMutex;
};

#endif // UNIVERSALPLAYER_H
