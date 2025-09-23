// Begin main.cpp

#include "appWindow.h"

int main(int argc, char** argv){

    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    QIcon appIcon(":/app-logo.svg");
    app.setWindowIcon(appIcon);

    try {
        QAppWindow appWindow;
        appWindow.show();
    } catch (const std::runtime_error &e) {
        qDebug() << e.what();
        return EXIT_FAILURE;
    }

    return app.exec();
}

// End main.cpp
