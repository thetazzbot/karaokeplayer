#include <QDebug>

#include "logger.h"
#include "eventcontroller.h"
#include "settings.h"
#include "playerbackgroundimage.h"

PlayerBackgroundImage::PlayerBackgroundImage()
{
    m_percentage = 0;
}

bool PlayerBackgroundImage::initFromSettings(const QString &)
{
    // Do we have a directory path or file?
    if ( pSettings->m_playerBackgroundObjects.isEmpty() )
        return false;

    // Load the first image
    loadNewImage();
    return true;
}

bool PlayerBackgroundImage::initFromFile(QIODevice &file)
{
    return false;
}

qint64 PlayerBackgroundImage::draw(KaraokePainter &p)
{
    // Transition
    if ( !m_newImage.isNull() )
    {
        if ( !m_currentImage.isNull() && performTransition(p) )
            return 0;

        // No transition or it is finished
        m_currentImage = m_newImage;
        m_newImage = QImage();
    }

    if ( !m_currentImage.isNull() )
        p.drawImage( QPoint(0,0), m_currentImage.scaled( p.size() ) );

    if ( pSettings->playerBackgroundTransitionDelay > 0 && m_lastUpdated.elapsed() > (int) pSettings->playerBackgroundTransitionDelay * 1000 )
        loadNewImage();

    return -1;
}

void PlayerBackgroundImage::loadNewImage()
{
    // Do we have more than one image?
    if ( pSettings->m_playerBackgroundObjects.size() > 1 )
    {
        // Next image
        pSettings->m_playerBackgroundLastObject++;

        if ( (int) pSettings->m_playerBackgroundLastObject >= pSettings->m_playerBackgroundObjects.size() )
            pSettings->m_playerBackgroundLastObject = 0;
    }
    else
        pSettings->m_playerBackgroundLastObject = 0;

    Logger::debug( "BgImage: show image %d", pSettings->m_playerBackgroundLastObject );

    // If current image is not set, fill up it. Otherwise fill up new image
    if ( !m_newImage.load( pSettings->m_playerBackgroundObjects[ pSettings->m_playerBackgroundLastObject ] ) )
    {
        m_newImage = QImage();
        pController->warning( QString("BgImage: error loading image %1") .arg( pSettings->m_playerBackgroundObjects[ pSettings->m_playerBackgroundLastObject ] ) );
    }

    m_lastUpdated = QTime::currentTime();
    m_lastUpdated.start();
    m_percentage = 0;
}

bool PlayerBackgroundImage::performTransition(KaraokePainter &p)
{
    // We want the whole transition to be done in 1.5 sec
    static const int TRANSITION = 250;
    int percstep = (pSettings->m_playerRenderMSecPerFrame * 100) / TRANSITION;

    // Paint the original image first so we have it
    p.drawImage( QPoint(0,0), m_currentImage.scaled( p.size() ) );

    // Now we have four rectangles; see how big they should be in current image
    int curwidth = (p.size().width() * m_percentage) / 200;
    int curheight = (p.size().height() * m_percentage) / 200;

    // and in the new image
    int newwidth = ( m_newImage.width() * m_percentage) / 200;
    int newheight = ( m_newImage.height() * m_percentage) / 200;

    // Left top corner
    p.drawImage( QRect( 0, 0, curwidth, curheight ), m_newImage, QRect( 0, 0, newwidth, newheight ) );

    // Left bottom corner
    p.drawImage( QRect( 0, p.size().height() - curheight, curwidth, p.size().height() ),
                 m_newImage,
                 QRect( 0, m_newImage.height() - newheight, newwidth, m_newImage.height() ) );

    // Right top corner
    p.drawImage( QRect( p.size().width() - curwidth, 0, p.size().width(), curheight ),
                 m_newImage,
                 QRect( m_newImage.width() - newwidth, 0, m_newImage.width(), newheight ) );

    // Right bottom corner
    p.drawImage( QRect( p.size().width() - curwidth, p.size().height() - curheight, p.size().width(), p.size().height() ),
                 m_newImage,
                 QRect( m_newImage.width() - newwidth, m_newImage.height() - newheight, m_newImage.width(), m_newImage.height() ) );

    m_percentage += percstep;

    if ( m_percentage >= 100 )
        return false;

    return true;
}
