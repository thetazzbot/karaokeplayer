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

#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QDockWidget>

#include "playerbutton.h"
#include "ui_playerwidget.h"
/*

// Taken from my other project, Karaoke Lyric Editor
class PlayerWidget : public QDockWidget, public Ui::PlayerWidget
{
    Q_OBJECT

	public:
		PlayerWidget(QWidget *parent = 0);
		~PlayerWidget();

	signals:
        void	event( int type );

	public slots:
		void	btn_playerStop();
		void	btn_playerPlayPause();
		void	btn_playerSeekForward();
		void	btn_playerSeekBackward();

        void	playUpdate();

	private:
		bool			m_sliderDown; // do not update position
};

*/
#endif // PLAYERWIDGET_H