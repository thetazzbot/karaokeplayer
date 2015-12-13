#include "playerbackgroundcolor.h"
#include "settings.h"
#include "karaokepainter.h"

PlayerBackgroundColor::PlayerBackgroundColor()
    : PlayerBackground()
{
}

bool PlayerBackgroundColor::initFromSettings(const QString &param)
{
    if ( param.isEmpty() )
        m_color = Settings::g()->playerBackgroundColor;
    else
        m_color = QColor( param );

    return m_color.isValid();
}

qint64 PlayerBackgroundColor::draw(KaraokePainter &p)
{
    p.fillRect( p.rect(), m_color );
    return -1;
}
