#include <QElapsedTimer>
#include <QDebug>
#include <QFile>

#include "player.h"

Player::Player()
    : QObject()
{
    m_audioPlayer = new QMediaPlayer();
    m_audioPlayer->setNotifyInterval( 100 );

    connect( m_audioPlayer, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(slotError(QMediaPlayer::Error)) );
    connect( m_audioPlayer, SIGNAL(positionChanged(qint64)), this, SIGNAL(musicTick(qint64)) );
    connect( m_audioPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)) );
    connect( m_audioPlayer, SIGNAL(mediaChanged(QMediaContent)), this, SLOT(slotMediaChanged(QMediaContent)) );
}


bool Player::load( QIODevice * musicfile )
{
    //if ( m_audioPlayer )
    //    stop();

    // We don't know if this succeeded or not until start() is called
    m_audioPlayer->setMedia(QMediaContent(), musicfile );

    return true;
}

void Player::stop()
{
}

qint64 Player::position()
{
    return m_audioPlayer->position();
}

void Player::slotError(QMediaPlayer::Error error)
{
    qDebug() << "error:" << error;
}


void Player::slotMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if ( status == QMediaPlayer::EndOfMedia )
        emit musicEnded();

    qDebug() << "slotMediaStatusChanged" << status;
}

void Player::slotMediaChanged(const QMediaContent &media)
{
    qDebug() << "slotMediaChanged" << media.canonicalUrl();
}

bool Player::start()
{
    //m_audioPlayer->setPosition( 270000 );
    m_audioPlayer->play();
    return true;
}
