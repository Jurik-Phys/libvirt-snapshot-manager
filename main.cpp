// Begin main.cpp

#include "appWindow.h"

int main(int argc, char** argv){

    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    QIcon appIcon(":/app-logo.svg");
    app.setWindowIcon(appIcon);

    QAppWindow* appWindow = nullptr;

    try {
        appWindow = new QAppWindow();
        appWindow->show();
    }
    catch (const std::runtime_error &e) {
        qDebug() << e.what();
        delete appWindow;
        return EXIT_FAILURE;
    }

    return app.exec();
}

// End main.cpp
