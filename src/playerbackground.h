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

        // Draws the background on the painter; the painter is filled up black. Returns time for the
        // next update when the image will change. -1 means the current image will never change.
        virtual qint64  draw( KaraokePainter& p ) = 0;

        // Current state notifications, may be reimplemented. Default implementation does nothing.
        virtual void    start();
        virtual void    pause( bool resuming );
        virtual void    stop();
};


#endif // PLAYERBACKGROUND_H
