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
        bool    initFromFile( QIODevice& ) { return false; }

        // Returns true if the background is static (draw function will only be called when lyrics change),
        // false if the background is dynamic (so lyrics would have to be re-rendered each time background changes)
        bool    isStatic() const { return true; }

        // For dynamic background, must return true if the background changed since the last call and draw must be called.
        // If the background did not change AND lyrics did not change, the renderer will just reuse the last image
        // Static background must always return false here.
        bool    needUpdate() const { return false; }

        // Draws the background on the image; the prior content of the image is undefined. If false is returned,
        // it is considered an error, and the whole playing process is aborted - use wisely
        bool    draw( qint64 time, QImage& target );

    private:
        QColor  m_color;
};

#endif // PLAYERBACKGROUNDCOLOR_H
