#include <QApplication>
#include <QScopedPointer>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set data for QSettings
    QCoreApplication::setOrganizationName("Ulduzsoft");
    QCoreApplication::setOrganizationDomain("ulduzsoft.com");
    QCoreApplication::setApplicationName("karaokeplayer");

    QScopedPointer<MainWindow> p (new MainWindow());
    p.data()->show();

    a.exec();
    return 0;
}
