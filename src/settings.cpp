#include "settings.h"

Settings * pSettings;


Settings::Settings()
{
    playerBackgroundColor = QColor( Qt::black );
    playerLyricsFont = QFont("arial");
    playerLyricsTextBeforeColor = QColor( Qt::red );
    playerLyricsTextAfterColor = QColor( Qt::blue );
    playerLyricsTextEachCharacter = true;
    cacheDir = "/home/tim/work/my/karaokeplayer/test/cache";
    convertMidiFiles = true;
    playerCDGbackgroundTransparent = true;
}
