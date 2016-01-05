#ifndef LABELSHOWHELP_H
#define LABELSHOWHELP_H

#include <QLabel>

class LabelShowHelp : public QLabel
{
    Q_OBJECT

    public:
        explicit LabelShowHelp(QWidget *parent = 0);

    signals:

    public slots:
        void    clicked();
};

#endif // LABELSHOWHELP_H
