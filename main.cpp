#include "mainwindow.h"

#include <QApplication>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    srand((unsigned) time(NULL));
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Planification des processus");
    w.show();
    return a.exec();
}
