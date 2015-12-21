#include <QApplication>
#include <QScopedPointer>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QScopedPointer<MainWindow> p (new MainWindow());
    p.data()->show();

    return a.exec();
}
