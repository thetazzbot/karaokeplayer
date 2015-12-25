#include "settings.h"
#include "currentstate.h"

CurrentState * pCurrentState;

CurrentState::CurrentState(QObject *parent) :
    QObject(parent)
{
    msecPerFrame = 1000 / pSettings->playerRenderFPS;
}
