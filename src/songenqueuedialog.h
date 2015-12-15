#ifndef SONGENQUEUEDIALOG_H
#define SONGENQUEUEDIALOG_H

#include <QDialog>
#include "ui_songenqueuedialog.h"

class SongEnqueueDialog : public QDialog, public Ui::SongEnqueueDialog
{
    Q_OBJECT

    public:
        explicit SongEnqueueDialog( QWidget *parent = 0 );

    private slots:
        void    browse();
        void    accept();
};

#endif // SONGENQUEUEDIALOG_H
