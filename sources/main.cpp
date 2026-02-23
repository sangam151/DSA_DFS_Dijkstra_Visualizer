#include <QApplication>
#include "mainWindow.h"
#include "GridView.h"
#include "PathAlgorithm.h"


int main(int argc, char *argv[])
{
    // Starting a new QApplication
    QApplication a(argc, argv);

    // Setup of the new Window
    MainWindow window;

    window.resize(500, 350);
    window.show();

    return a.exec();
}
