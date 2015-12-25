#include <QFileDialog>

#include "songenqueuedialog.h"
#include "actionhandler.h"

SongEnqueueDialog::SongEnqueueDialog(QWidget *parent) :
    QDialog(parent), Ui::SongEnqueueDialog()
{
    setupUi(this);
    connect( btnBrowse, SIGNAL(clicked()), this, SLOT(browse()) );
}

void SongEnqueueDialog::browse()
{
    QString file = QFileDialog::getOpenFileName( 0, "Music file", "/home/tim/work/my/karaokeplayer/test/", "*.*");

    if ( !file.isEmpty() )
        leFile->setText( file );
}

void SongEnqueueDialog::accept()
{
    if ( leFile->text().isEmpty() || leSinger->text().isEmpty() )
        return;

    QDialog::accept();
}
