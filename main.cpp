// Begin main.cpp

#include "appWindow.h"

int main(int argc, char** argv){

    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    QAppWindow appWindow;
    appWindow.show();

    int res = app.exec();

    return res;
}

// End main.cpp
