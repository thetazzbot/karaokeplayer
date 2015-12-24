#include "playerlyrics.h"
#include "settings.h"
#include "karaokepainter.h"


PlayerLyrics::PlayerLyrics()
{
    m_timeDelay = pSettings->playerMusicLyricDelay;
}

PlayerLyrics::~PlayerLyrics()
{
}

bool PlayerLyrics::draw(KaraokePainter &p)
{
    qint64 origtime = p.time();

    // Prevent time going below zero
    p.setTime( qMax( 0LL, origtime + m_timeDelay ) );

    if ( !render( p ) )
        return false;

    p.setTime( origtime );
    m_lastRenderedTime = origtime;

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
