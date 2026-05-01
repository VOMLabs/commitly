#include <QApplication>
#include "mainwindow.h"
#include "theme.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Commitly");
    a.setApplicationVersion("0.1.0");
    a.setOrganizationName("VOMLabs");
    a.setOrganizationDomain("vomlabs.com");

    commitly::applyDarkTheme(&a);

    commitly::MainWindow w;
    w.show();

    return a.exec();
}
