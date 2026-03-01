// Begin nodeInfoProvider.cpp

#include "nodeInfoProvider.h"
#include <QProcess>
#include <QEventLoop>


NodeInfoProvider::NodeInfoProvider(QObject* parent) : QObject(parent){
    getNodeInfo();
}

NodeInfoProvider::~NodeInfoProvider(){

}

void NodeInfoProvider::getNodeInfo(){

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");
    QProcess process;
    process.setProcessEnvironment(env);
    QEventLoop loop;

    QObject::connect(&process, &QProcess::finished,
            [&](){
                QString output = process.readAllStandardOutput();
                QStringList outputLines = output.split('\n',Qt::SkipEmptyParts);
                long int rawMemValue = 0;
                QString memUnit = "";

                for (int i = 0; i < outputLines.size(); ++i){
                    QStringList line = outputLines[i].split(":");
                    if (line.first() == "Memory size"){
                        QString rawMemString = line.value(1).simplified();
                        QStringList memData = rawMemString.split(" ");
                        rawMemValue = memData.value(0).toInt();
                        memUnit = memData.value(1);
                    }

                    if (line.first() == "CPU(s)"){
                        m_cpuCount = line.last().simplified().toInt();
                    }
                }

                m_memSizeBytes = getRamBytes(rawMemValue, memUnit);
                loop.quit();
            });

    process.start("virsh", {"--connect=" + m_libVirtConnectURI, "nodeinfo"});
    loop.exec();
}

long int NodeInfoProvider::getRamBytes(const long int& memValue,
                                                          const QString& units){
    long int bytesRam;
    QString memUnits = units;

    // *** Передано число без единиц измерения (байты) *** //
    //     В этом случае pars.first() == parts.last()      //
    if (memUnits.size() == 0){
        memUnits = "bytes";
    }

    QStringList s1024BaseUnits = {   "k",   "M",   "G",   "T" };
    QStringList l1024BaseUnits = { "KiB", "MiB", "GiB", "TiB" };
    QStringList v1000BaseUnits = {  "KB",  "MB",  "GB",  "TB" };

    if (memUnits == "b" || memUnits == "bytes"){
        bytesRam = memValue;
    }

    if (memUnits == "KB" ){
        bytesRam = memValue * 1000;
    }

    if (memUnits == "MB"){
        bytesRam = memValue * 1000 * 1000;
    }

    if (memUnits == "GB"){
        bytesRam = memValue * 1000 * 1000 * 1000;
    }

    if (memUnits == "TB"){
        bytesRam = memValue * 1000 * 1000 * 1000 * 1000;
    }

    if (memUnits == "k" || memUnits == "KiB"){
        bytesRam = memValue * 1024;
    }

    if (memUnits == "M" || memUnits == "MiB"){
        bytesRam == memValue * 1024 * 1024;
    }

    if (memUnits == "G" || memUnits == "GiB"){
        bytesRam == memValue * 1024 * 1024 * 1024;
    }

    if (memUnits == "T" || memUnits == "TiB"){
        bytesRam == memValue * 1024 * 1024 * 1024 * 1024;
    }

    return bytesRam;
}

long int NodeInfoProvider::memorySizeBytes(){
    return m_memSizeBytes;
}

long int NodeInfoProvider::logicalCpuCount(){
    return m_cpuCount;
}

// End nodeInfoProvider.cpp
