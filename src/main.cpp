#include <QApplication>
#include <QScopedPointer>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QScopedPointer<MainWindow> p (new MainWindow());
    pMainWindow = p.data();
    pMainWindow->show();

    return a.exec();
}
