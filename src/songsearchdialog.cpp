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
        SongSearchResult( QTreeWidget *view, int id_,  const QString& artist_, const QString& title_ )
            : QTreeWidgetItem( view)
        {
            id = id_;
            artist = artist_;
            title = title_;
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
                    return artist;
                else
                    return title;
            }

            return QVariant();
        }

        int         id;
        QString     artist;
        QString     title;
};


SongSearchDialog::SongSearchDialog(QWidget *parent) :
    QDialog(parent), Ui::SongSearchDialog()
{
    setupUi(this);
    connect( btnSearch, SIGNAL(clicked()), this, SLOT(buttonSearch()) );
    buttonBox->button( QDialogButtonBox::Ok )->setText( tr("Add to queue") );

    treeWidget->header()->setSectionResizeMode( QHeaderView::Stretch );
}

int SongSearchDialog::selectedSongId() const
{
    SongSearchResult * r = (SongSearchResult *) treeWidget->currentItem();

    if ( !r )
        return 0;

    return r->id;
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

    QList< SongDatabase::SearchResult > results;

    if ( !pSongDatabase->search( leSearch->text(), results ) )
        return;

    Q_FOREACH( const SongDatabase::SearchResult& res, results )
        new SongSearchResult( treeWidget, res.id, res.artist, res.title );
}
