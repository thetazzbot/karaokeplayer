/**************************************************************************
 *  Karlyriceditor - a lyrics editor and CD+G / video export for Karaoke  *
 *  songs.                                                                *
 *  Copyright (C) 2009-2013 George Yunaev, support@ulduzsoft.com          *
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

#include <QTime>
#include <QTimer>

#include "playerwidget.h"
#include "currentstate.h"
#include "actionhandler.h"


PlayerWidget::PlayerWidget(QWidget *parent)
	: QDockWidget(parent), Ui::PlayerWidget()
{
	setupUi(this);
    setFloating( true );
    setAttribute( Qt::WA_DeleteOnClose );

	// Set up icons
	btnFwd->setPixmap( QPixmap(":images/dryicons_forward.png") );
	btnPausePlay->setPixmap( QPixmap(":images/dryicons_play.png") );
	btnRew->setPixmap( QPixmap(":images/dryicons_rewind.png") );
	btnStop->setPixmap( QPixmap(":images/dryicons_stop") );

	// Connect button slots
	connect( btnFwd, SIGNAL( clicked() ), this, SLOT(btn_playerSeekForward()) );
	connect( btnRew, SIGNAL( clicked() ), this, SLOT(btn_playerSeekBackward()) );
	connect( btnPausePlay, SIGNAL( clicked() ), this, SLOT(btn_playerPlayPause()) );
	connect( btnStop, SIGNAL( clicked() ), this, SLOT(btn_playerStop()) );

    seekBar->setRange( 0, 100 );
    btnPausePlay->setEnabled( true );

    playUpdate();
}

void PlayerWidget::playUpdate()
{
    if ( pCurrentState->playerState != CurrentState::PLAYERSTATE_STOPPED )
    {
        int total = qMax( 1, (int) pCurrentState->playerDuration );
        int pos = pCurrentState->playerPosition;
        int percent = (pos * 100) / total;
        int remaining = total - pos;

        lblInfo->setText( tr("%1 by %2") .arg(pCurrentState->playerSong.title) .arg( pCurrentState->playerSong.singer) );
        lblTimeCur->setText( tr("<b>%1</b>") .arg( tickToString( pos ) ) );
        lblTimeRemaining->setText( tr("<b>-%1</b>") .arg( tickToString( remaining ) ) );
        seekBar->setValue( percent );

        btnFwd->setEnabled( true );
        btnRew->setEnabled( true );
        btnStop->setEnabled( true );
    }
    else
    {
        lblInfo->setText( "Nothing is played" );
        lblTimeCur->setText( "<b>---</b>" );
        lblTimeRemaining->setText( "<b>---</b>" );
        seekBar->setValue( 0 );

        btnFwd->setEnabled( false );
        btnRew->setEnabled( false );
        btnStop->setEnabled( false );
    }

    QTimer::singleShot( 500, this, SLOT(playUpdate()) );
}

QString PlayerWidget::tickToString( qint64 tickvalue )
{
	int minute = tickvalue / 60000;
	int second = (tickvalue-minute*60000)/1000;
	int msecond = (tickvalue-minute*60000-second*1000) / 100; // round milliseconds to 0-9 range}

	QTime ct( 0, minute, second, msecond );
	return ct.toString( "mm:ss.z" );
}

void PlayerWidget::btn_playerStop()
{
    pActionHandler->cmdAction( ActionHandler::ACTION_PLAYER_STOP );
}

void PlayerWidget::btn_playerPlayPause()
{
    if ( pCurrentState->playerState == CurrentState::PLAYERSTATE_STOPPED )
        pActionHandler->cmdAction( ActionHandler::ACTION_PLAYER_START );
    else
        pActionHandler->cmdAction( ActionHandler::ACTION_PLAYER_PAUSERESUME );
}

void PlayerWidget::btn_playerSeekForward()
{
    pActionHandler->cmdAction( ActionHandler::ACTION_PLAYER_SEEK_FORWARD );
}

void PlayerWidget::btn_playerSeekBackward()
{
    pActionHandler->cmdAction( ActionHandler::ACTION_PLAYER_SEEK_BACK );
}
