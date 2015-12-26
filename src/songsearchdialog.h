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

#ifndef SONGSEARCHDIALOG_H
#define SONGSEARCHDIALOG_H

#include <QDialog>
#include "ui_songsearchdialog.h"

class SongSearchDialog : public QDialog, public Ui::SongSearchDialog
{
    Q_OBJECT

    public:
        explicit SongSearchDialog(QWidget *parent = 0);

        // returns 0 if nothing is selected, which shouldn't happen
        int selectedSongId() const;

    public slots:
        void    accept();
        void    buttonSearch();

};

#endif // SONGSEARCHDIALOG_H
