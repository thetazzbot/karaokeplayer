#include "playerbackgroundcolor.h"
#include "settings.h"

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

bool PlayerBackgroundColor::draw( qint64 time, QImage& target )
{
    target.fill( m_color );
    return true;
}
