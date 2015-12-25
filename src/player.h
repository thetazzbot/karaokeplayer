#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QString>
#include <QMediaPlayer>


// A player has
class Player : public QObject
{
    Q_OBJECT

    public:
        Player();
        ~Player();

        // Load the file, which could be:
        // - A music file (we need to find a matching lyrics file);
        // - A ZIP archive (we should extract both music and matching lyrics file from it);
        // - A compound file such as KFN;
        void    load( const QString& musicfile );

        bool    play();

        void    stop();
        void    pause();
        void    seekTo( qint64 time );

        qint64  position();
        qint64  duration();

        QString errorMsg() const { return m_errorMsg; }

    signals:
        void    finished();

    private slots:
        void	slotError(QMediaPlayer::Error error);
        void    slotMediaStatusChanged(QMediaPlayer::MediaStatus status);
        void    volumeDown();
        void    volumeUp();

    private:
        QString         m_errorMsg;

        QMediaPlayer  * m_player;

        bool            m_loaded_started;
};

#endif // PLAYER_H
