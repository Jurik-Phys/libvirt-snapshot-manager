// Begin main.cpp

#include <QCoreApplication>
#include "osinfoloader.h"

int main(int argc, char** argv){

    QCoreApplication app(argc, argv);

    OsInfoLoader osInfoLoader;
    QJsonDocument libOsInfoJsonDoc;

//  QString oldUrl;
//  oldUrl = "https://releases.pagure.org/libosinfo/osinfo-db-20170121.tar.xz";
//  libOsInfoJsonDoc = osInfoLoader.getLibOsInfoJson(oldUrl);

    // *** Получение информации из последнего релиза *** //
    libOsInfoJsonDoc = osInfoLoader.getLibOsInfoJson();
    if (libOsInfoJsonDoc.isEmpty()){
        return EXIT_FAILURE;
    }

    QString fullFileName = QDir::tempPath() + "/" + "libOsInfo.json";
    osInfoLoader.writeLibOsInfoJsonToFile(libOsInfoJsonDoc, fullFileName);
    return 0;
}

// End main.cpp
