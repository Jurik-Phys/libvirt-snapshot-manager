// Begin main.cpp

#include <QCoreApplication>
#include "osinfoloader.h"

int main(int argc, char** argv){

    QCoreApplication app(argc, argv);

    OsInfoLoader osInfoLoader;
    QJsonDocument libOsInfoJsonDoc;

    // *** Получение информации из последнего релиза *** //
    libOsInfoJsonDoc = osInfoLoader.getLibOsInfoJson();
    if (libOsInfoJsonDoc.isEmpty()){
        return EXIT_FAILURE;
    }

    QString fullFileName = QDir::tempPath() + "/" + "libOsInfo.json";
    osInfoLoader.writeLibOsInfoJsonToFile(libOsInfoJsonDoc, fullFileName);
    qDebug() << "[II] libOsInfo write to" << fullFileName;
    return 0;
}

// End main.cpp
