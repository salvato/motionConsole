#include <QCoreApplication>
#include "motionthread.h"

int
main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    motionThread myThread;
    return a.exec();
}
