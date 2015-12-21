#include <QElapsedTimer>
#include <QDebug>
#include <QFile>

#include "player.h"

Player::Player()
    : QObject()
{
    m_player = new QMediaPlayer( this );

    connect( m_player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(slotError(QMediaPlayer::Error)) );
    connect( m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)) );
    connect( m_player, SIGNAL(mediaChanged(QMediaContent)), this, SLOT(slotMediaChanged(QMediaContent)) );
}

Player::~Player()
{
}

void Player::load(const QString &musicfile)
{
    m_loaded_started = false;
    m_player->setMedia( QMediaContent( QUrl::fromLocalFile( musicfile ) ) );
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

    if ( status == QMediaPlayer::LoadedMedia )
    {
        if ( m_loaded_started )
            m_player->play();
        else
            m_loaded_started = true;
    }

    qDebug() << "slotMediaStatusChanged" << status;
}

void Player::slotMediaChanged(const QMediaContent &media)
{
    qDebug() << "slotMediaChanged" << media.canonicalUrl();
}

bool Player::play()
{
    if ( m_loaded_started )
        m_player->play();
    else
        m_loaded_started = true;

    return true;
}
