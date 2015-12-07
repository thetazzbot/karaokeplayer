#include "playerlyrics.h"

PlayerLyrics::PlayerLyrics()
{
}

PlayerLyrics::~PlayerLyrics()
{
}

bool PlayerLyrics::draw(qint64 time, QImage &target)
{
    if ( !render( time, target) )
        return false;

    m_lastRenderedTime = time;
    return true;
}
