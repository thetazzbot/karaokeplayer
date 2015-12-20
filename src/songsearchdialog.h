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
