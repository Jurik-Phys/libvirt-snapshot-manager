// Begin osInfoProvider.cpp

#include "osInfoProvider.h"

OsInfoProvider::OsInfoProvider(QObject* parent) : QObject(parent) {
}

OsInfoProvider::~OsInfoProvider(){

}

void OsInfoProvider::dataInit(){
    this->loadFromResources();

    // qDebug() << "[II] libOsInfoVersion" << m_libOsInfoVersion;
    //
    // for (int i = 0; i < m_osInfoData.size(); ++i){
    //     qDebug() << "= = =" << i << "= = =";
    //     qDebug() << ".id     " << m_osInfoData[i].id;
    //     qDebug() << ".name   " << m_osInfoData[i].name;
    //     qDebug() << ".vendor " << m_osInfoData[i].vendor;
    //     qDebug() << ".ram    " << m_osInfoData[i].ram;
    // }
}

OsInfo OsInfoProvider::getOsInfo(const QString& osId){
    if (m_osInfoData.isEmpty()){
        this->dataInit();
    }

    OsInfo res;
    // *** В QVector<OsInfo> m_osInfoData менее 2500 элементов,  *** //
    //     линейный поиск прост в реализации не займёт много времени //
    for (int i = 0; i < m_osInfoData.size(); ++i){
        if (m_osInfoData[i].id == osId){
            res = m_osInfoData[i];
        }
    }
    return res;
}

void OsInfoProvider::loadFromResources(){
    QFile file(":/libOsInfo.json");

    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "[EE] Error open JSON from resources";
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()){
        qDebug() << "[EE] Invalid JSON";
    }

    QJsonArray osInfoJsonArray = doc.object()["osInfo"].toArray();
    // m_osInfoData.reserve(osInfoJsonArray.size());

    for (int i = 0; i < osInfoJsonArray.size(); ++i){
        QJsonObject obj = osInfoJsonArray[i].toObject();
        OsInfo os;

        os.id = obj["id"].toString();
        os.name = obj["name"].toString();
        os.vendor = obj["vendor"].toString();
        os.ram = obj["ram"].toString();

        m_osInfoData.push_back(os);
    }

    m_libOsInfoVersion = doc.object()["libOsInfoVersion"].toInt();
}
// End osInfoProvider.cpp
