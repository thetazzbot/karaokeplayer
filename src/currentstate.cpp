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

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QApplication>

#include "logger.h"
#include "actionhandler.h"
#include "settings.h"
#include "currentstate.h"

CurrentState * pCurrentState;

CurrentState::CurrentState(QObject *parent)
    : QObject(parent)
{
    msecPerFrame = 1000 / pSettings->playerRenderFPS;
    playerVolume = 0;

    m_playerBackgroundLastObject = 0;
    playerBackgroundLastVideoPosition = 0;

    // Validate settings
    if ( pSettings->playerBackgroundType == Settings::BACKGROUND_TYPE_IMAGE || pSettings->playerBackgroundType == Settings::BACKGROUND_TYPE_VIDEO )
    {
        loadBackgroundObjects();

        if ( m_playerBackgroundObjects.isEmpty() )
        {
            pActionHandler->error( tr("No %1 was found; disabling background") .arg( pSettings->playerBackgroundType == Settings::BACKGROUND_TYPE_VIDEO ? "videos" : "images") );
            pSettings->playerBackgroundType = Settings::BACKGROUND_TYPE_NONE;
        }
    }
}

void CurrentState::loadBackgroundObjects()
{
    m_playerBackgroundObjects.clear();

    QStringList extensions;

    if ( pSettings->playerBackgroundType == Settings::BACKGROUND_TYPE_IMAGE )
        extensions << "*.jpg" << "*.png" << "*.gif" << "*.bmp";
    else
        extensions << "*.avi" << "*.mkv" << "*.mp4" << "*.3gp" << "*.mov";

    // Entries in playerBackgroundObjects could be files or directories
    for ( int i = 0; i < pSettings->playerBackgroundObjects.size(); i++ )
    {
        QFileInfo finfo( pSettings->playerBackgroundObjects[i] );

        if ( finfo.isDir() )
        {
            QFileInfoList list = QDir( pSettings->playerBackgroundObjects[i] ).entryInfoList( extensions, QDir::Files | QDir::NoDotAndDotDot );

            foreach ( const QFileInfo& f, list )
                m_playerBackgroundObjects.push_back( f.absoluteFilePath() );
        }
        else if ( finfo.isFile() )
            m_playerBackgroundObjects.push_back( finfo.absoluteFilePath() );
    }

    Logger::debug( "Background objects: %d loaded", m_playerBackgroundObjects.size() );

    // Load temporary parameters and validate them
    QSettings settings;

    // Do we still have this object?
    QString lastobject = settings.value( "temp/playerBackgroundLastObject", "" ).toString();
    int lastindex;

    if ( lastobject.isEmpty() || (lastindex = m_playerBackgroundObjects.indexOf( lastobject )) == -1 )
    {
        // Nope; reset the values
        m_playerBackgroundLastObject = 0;
        playerBackgroundLastVideoPosition = 0;
    }
    else
    {
        // Yes; read the rest values
        m_playerBackgroundLastObject = lastindex;
        playerBackgroundLastVideoPosition = settings.value( "temp/playerBackgroundLastVideoPosition", 0 ).toInt();
    }
}

void CurrentState::saveTempData()
{
    // Load temporary parameters and validate them
    QSettings settings;

    if ( !m_playerBackgroundObjects.isEmpty() )
    {
        settings.setValue( "temp/playerBackgroundLastObject", currentBackgroundObject() );
        settings.setValue( "temp/playerBackgroundLastVideoPosition", playerBackgroundLastVideoPosition );
    }
}

QString CurrentState::currentBackgroundObject()
{
    if ( m_playerBackgroundObjects.isEmpty() )
        return "";

    return m_playerBackgroundObjects[ m_playerBackgroundLastObject ];
}

void CurrentState::nextBackgroundObject()
{
    pCurrentState->playerBackgroundLastVideoPosition = 0;
    m_playerBackgroundLastObject++;

    if ( (int) m_playerBackgroundLastObject >= m_playerBackgroundObjects.size() )
        m_playerBackgroundLastObject = 0;
}
