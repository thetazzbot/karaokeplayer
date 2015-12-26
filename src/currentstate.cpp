#include <QApplication>
#include "settings.h"
#include "currentstate.h"

CurrentState * pCurrentState;

CurrentState::CurrentState(QObject *parent)
    : QObject(parent)
{
    msecPerFrame = 1000 / pSettings->playerRenderFPS;
    modeFullscreen = false;

    for ( int i = 1; i < qApp->arguments().size(); i++ )
    {
        if ( qApp->arguments()[i] == "-fs" )
            modeFullscreen = true;
    }

    modeFullscreen = true;
}
