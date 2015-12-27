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

#include "songsearchdialog.h"
#include "songdatabase.h"

class SongSearchResult : public QTreeWidgetItem
{
    public:
        SongSearchResult( QTreeWidget *view, const SongDatabaseInfo& info_ )
            : QTreeWidgetItem( view)
        {
            info = info_;
        }

        int columnCount() const
        {
            return 2;
        }

        QVariant data(int column, int role) const
        {
            if ( role == Qt::DisplayRole )
            {
                if ( column == 0 )
                    return info.artist;
                else
                    return info.title;
            }

            return QVariant();
        }

        SongDatabaseInfo    info;
};


SongSearchDialog::SongSearchDialog(QWidget *parent) :
    QDialog(parent), Ui::SongSearchDialog()
{
    setupUi(this);
    connect( btnSearch, SIGNAL(clicked()), this, SLOT(buttonSearch()) );
    buttonBox->button( QDialogButtonBox::Ok )->setText( tr("Add to queue") );

    treeWidget->header()->setSectionResizeMode( QHeaderView::Stretch );
}

SongDatabaseInfo * SongSearchDialog::selectedSong() const
{
    SongSearchResult * r = (SongSearchResult *) treeWidget->currentItem();

    if ( !r )
        return 0;

    return &r->info;
}

void SongSearchDialog::accept()
{
    if ( treeWidget->currentItem() == 0 )
        return;

    QDialog::accept();
}

void SongSearchDialog::buttonSearch()
{
    if ( leSearch->text().isEmpty() )
        return;

    //if ( listResults->currentItem() )
    treeWidget->clear();

    QList< SongDatabaseInfo > results;

    if ( !pSongDatabase->search( leSearch->text(), results ) )
        return;

    Q_FOREACH( const SongDatabaseInfo& res, results )
        new SongSearchResult( treeWidget, res );
}
