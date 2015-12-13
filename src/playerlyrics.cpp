#include "playerlyrics.h"
#include "karaokepainter.h"


PlayerLyrics::PlayerLyrics()
{
}

PlayerLyrics::~PlayerLyrics()
{
}

bool PlayerLyrics::draw(KaraokePainter &p)
{
    if ( !render( p ) )
        return false;

    m_lastRenderedTime = p.time();

    return true;
}
