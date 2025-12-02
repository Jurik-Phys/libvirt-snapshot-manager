// Begin main.cpp

#include <QCoreApplication>
#include "osinfoloader.h"

int main(int argc, char** argv){

    QCoreApplication app(argc, argv);

    OsInfoLoader osInfoLoader;

    osInfoLoader.run();

    return 0;
}

// End main.cpp
