#include <QElapsedTimer>
#include <QDebug>
#include <QFile>

#include "player.h"

Player::Player()
    : QObject()
{
    m_player = new QMediaPlayer();

    connect( m_player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(slotError(QMediaPlayer::Error)) );
    connect( m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)) );
    connect( m_player, SIGNAL(mediaChanged(QMediaContent)), this, SLOT(slotMediaChanged(QMediaContent)) );
}


void Player::load( QIODevice * musicfile )
{
    // We don't know if this succeeded or not until start() is called
    m_player->setMedia(QMediaContent(), musicfile );
}

void Player::stop()
{
    m_player->stop();
}

void Player::pause()
{
    m_player->pause();
}

void Player::seekTo(qint64 time)
{
    m_player->setPosition( time );
}

qint64 Player::position()
{
    return m_player->position();
}

qint64 Player::duration()
{
    return m_player->duration();
}

void Player::slotError(QMediaPlayer::Error error)
{
    qDebug() << "error:" << error;
}


void Player::slotMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if ( status == QMediaPlayer::EndOfMedia )
        emit finished();

    qDebug() << "slotMediaStatusChanged" << status;
}

void Player::slotMediaChanged(const QMediaContent &media)
{
    qDebug() << "slotMediaChanged" << media.canonicalUrl();
}

bool Player::play()
{
    m_player->play();
    return true;
}
