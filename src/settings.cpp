#include "settings.h"

Settings * pSettings;


Settings::Settings()
{
    playerBackgroundType = BACKGROUND_TYPE_VIDEO;
    playerBackgroundObjects << "/home/tim/work/my/karaokeplayer/test/bg0.mp4" << "/home/tim/work/my/karaokeplayer/test/bg1.mp4";

    m_playerBackgroundLastObject = 0;
    m_playerBackgroundLastVideoTime = 0;

    playerBackgroundColor = QColor( Qt::black );
    playerLyricsFont = QFont("arial");
    playerLyricsTextBeforeColor = QColor( Qt::red );
    playerLyricsTextAfterColor = QColor( Qt::blue );
    playerLyricsTextEachCharacter = true;
    cacheDir = "/home/tim/work/my/karaokeplayer/test/cache";
    convertMidiFiles = true;
    playerCDGbackgroundTransparent = true;

    queueAddNewSingersNext = false;
    queueFilename = "/home/tim/work/my/karaokeplayer/test/queue.dat";

    songdbFilename = "/home/tim/work/my/karaokeplayer/test/karaoke.db";
    songPathPrefix = "/mnt/karaoke";
}
