#ifndef PLAYERBACKGROUND_H
#define PLAYERBACKGROUND_H

#include <QIODevice>

class KaraokePainter;

// An abstract background renderer class
class PlayerBackground
{
    public:
        PlayerBackground();
        virtual ~PlayerBackground();

        // Background could be initialized either from the settings (by callign initFromSettings() or from
        // a specific file/QIODevice - for example for KFN files. If the background cannot be initialized
        // a specific way, it must return an error.
        virtual bool    initFromSettings( const QString& param = "" ) = 0;
        virtual bool    initFromFile( QIODevice& file ) = 0;

        // Returns true if the background is static (draw function will only be called when lyrics change),
        // false if the background is dynamic (so lyrics would have to be re-rendered each time background changes)
        virtual bool    isStatic() const = 0;

        // For dynamic background, must return true if the background changed since the last call and draw must be called.
        // If the background did not change AND lyrics did not change, the renderer will just reuse the last image
        // Static background must always return false here.
        virtual bool    needUpdate() const = 0;

        // Draws the background on the image; the prior content of the image is undefined. If false is returned,
        // it is considered an error, and the whole playing process is aborted - use wisely
        virtual bool    draw( KaraokePainter& p ) = 0;
};


#endif // PLAYERBACKGROUND_H
