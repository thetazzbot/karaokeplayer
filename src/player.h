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

        // Load the file, which could be:
        // - A music file (we need to find a matching lyrics file);
        // - A ZIP archive (we should extract both music and matching lyrics file from it);
        // - A compound file such as KFN;
        bool    load( QIODevice * musicfile );

        bool    play();

        void    stop();
        void    pause();
        void    seekTo( qint64 time );

        qint64  position();
        qint64  duration();

        QString errorMsg() const { return m_errorMsg; }

    signals:
        void    musicEnded();

    private slots:
        void	slotError(QMediaPlayer::Error error);
        //void	slotPositionChanged(qint64 position);
        void    slotMediaStatusChanged(QMediaPlayer::MediaStatus status);
        void    slotMediaChanged(const QMediaContent & media);

    private:
        QString         m_errorMsg;

        QMediaPlayer  * m_audioPlayer;
};

#endif // PLAYER_H
