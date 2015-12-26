#include <QElapsedTimer>
#include <QDebug>
#include <QFile>

#include "actionhandler.h"
#include "currentstate.h"
#include "player.h"

Player::Player()
    : QObject()
{
    m_player = new QMediaPlayer();

    connect( m_player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(slotError(QMediaPlayer::Error)) );
    connect( m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)) );

    connect( pActionHandler, SIGNAL(playerVolumeDown()), this, SLOT(volumeDown()) );
    connect( pActionHandler, SIGNAL(playerVolumeUp()), this, SLOT(volumeUp()) );
}

Player::~Player()
{
    m_player->deleteLater();
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

void Player::volumeDown()
{
    int newvolume = qMax( m_player->volume() - 10, 0 );
    m_player->setVolume( newvolume );
    pCurrentState->playerVolume = newvolume;
}

void Player::volumeUp()
{
    int newvolume = qMin( m_player->volume() + 10, 100 );
    m_player->setVolume( newvolume );
    pCurrentState->playerVolume = newvolume;
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
        pCurrentState->playerDuration = m_player->duration();
        pCurrentState->playerVolume = m_player->volume();

        if ( m_loaded_started )
            m_player->play();
        else
            m_loaded_started = true;
    }

    qDebug() << "slotMediaStatusChanged" << status;
}

bool Player::play()
{
    if ( m_loaded_started )
        m_player->play();
    else
        m_loaded_started = true;

    return true;
}
