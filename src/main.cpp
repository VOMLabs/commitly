#include <QApplication>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("commitly");
    app.setApplicationVersion("0.1.0");

    // Set default font
    QFont defaultFont = app.font();
    defaultFont.setPointSize(10);
    app.setFont(defaultFont);

    MainWindow window;
    window.setWindowTitle("commitly");
    window.show();

    return app.exec();
}