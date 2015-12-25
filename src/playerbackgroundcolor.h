#ifndef PLAYERBACKGROUNDCOLOR_H
#define PLAYERBACKGROUNDCOLOR_H

#include <QColor>

#include "playerbackground.h"

class PlayerBackgroundColor : public PlayerBackground
{
    public:
        PlayerBackgroundColor();

        // Background could be initialized either from the settings (by callign initFromSettings() or from
        // a specific file/QIODevice - for example for KFN files. If the background cannot be initialized
        // a specific way, it must return an error.
        bool    initFromSettings( const QString& param = "" );

        // Color background doesn't support files
        bool    initFromFile( QIODevice * ) { return false; }

        // Draws the background on the image; the prior content of the image is undefined. If false is returned,
        // it is considered an error, and the whole playing process is aborted - use wisely
        qint64   draw( KaraokePainter& p );

    private:
        QColor  m_color;
};

#endif // PLAYERBACKGROUNDCOLOR_H
