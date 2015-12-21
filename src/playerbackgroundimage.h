#ifndef PLAYERBACKGROUNDIMAGE_H
#define PLAYERBACKGROUNDIMAGE_H

#include <QTime>
#include <QImage>

#include "karaokepainter.h"
#include "playerbackground.h"

// Background for image or slideshow
class PlayerBackgroundImage : public PlayerBackground
{
    public:
        PlayerBackgroundImage();

        // Background could be initialized either from the settings (by callign initFromSettings() or from
        // a specific file/QIODevice - for example for KFN files. If the background cannot be initialized
        // a specific way, it must return an error.
        virtual bool    initFromSettings( const QString& param = "" );
        virtual bool    initFromFile( QIODevice& file );

        // Draws the background on the image; the prior content of the image is undefined. If false is returned,
        // it is considered an error, and the whole playing process is aborted - use wisely
        virtual qint64  draw( KaraokePainter& p );

    private:
        void    loadNewImage();
        bool    performTransition( KaraokePainter& p );

        // Last time the image was updated
        QTime   m_lastUpdated;

        // If transition is in progress, it is from current to new; otherwise new is always invalid
        QImage  m_currentImage;
        QImage  m_newImage;

        // For transition
        int     m_percentage;
};

#endif // PLAYERBACKGROUNDIMAGE_H
