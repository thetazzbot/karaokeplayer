/**************************************************************************
 *  Ulduzsoft Karaoke PLayer - cross-platform desktop karaoke player      *
 *  Copyright (C) 2015-2016 George Yunaev, support@ulduzsoft.com          *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *																	      *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

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
}
