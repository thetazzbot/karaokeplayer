#include "settings.h"

Settings *Settings::g()
{
    static Settings * d;

    if ( d == 0 )
        d = new Settings();

    return d;
}


Settings::Settings()
{
    playerBackgroundColor = QColor( Qt::black );
    playerLyricsFont = QFont("arial");
    playerLyricsTextBeforeColor = QColor( Qt::red );
    playerLyricsTextAfterColor = QColor( Qt::blue );
    playerLyricsTextEachCharacter = true;
    cacheDir = "/home/tim/work/my/karaokeplayer/test/cache";
    convertMidiFiles = true;
}
