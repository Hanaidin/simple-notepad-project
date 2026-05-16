#include "main_window.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("AUCA");
    QApplication::setApplicationName("Simple Notepad Project");

    MainWindow window;
    window.show();

    return app.exec();
}
