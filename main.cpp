// Begin main.cpp

#include "appWindow.h"

int main(int argc, char** argv){

    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    QIcon appIcon(":/app-logo.svg");
    app.setWindowIcon(appIcon);

    QAppWindow appWindow;

    if (appWindow.checkExternalVmUtilities()){
        appWindow.show();
        int res = app.exec();
        return res;
    }
    else {
        return EXIT_FAILURE;
    }
}

// End main.cpp
