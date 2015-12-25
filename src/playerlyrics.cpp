#include "playerlyrics.h"
#include "settings.h"
#include "karaokepainter.h"
#include "currentstate.h"

PlayerLyrics::PlayerLyrics()
{
    m_timeDelay = pSettings->playerMusicLyricDelay;
}

PlayerLyrics::~PlayerLyrics()
{
}

bool PlayerLyrics::draw(KaraokePainter &p)
{
    qint64 time = qMax( 0, pCurrentState->playerPosition + m_timeDelay );

    if ( !render( p, time ) )
        return false;

    m_lastRenderedTime = pCurrentState->playerPosition;
    return true;
}

void PlayerLyrics::setDelay(int delayms)
{
    m_timeDelay = delayms;
}

int PlayerLyrics::delay() const
{
    return m_timeDelay;
}
