// Begin osInfoProvider.cpp

#include "osInfoProvider.h"

OsInfoProvider::OsInfoProvider(QObject* parent) : QObject(parent) {
        m_osInfoLoader = new OsInfoLoader();
}

OsInfoProvider::~OsInfoProvider(){
        delete m_osInfoLoader;
}

void OsInfoProvider::dataInit(){
    QVector<OsInfo> osInfoDataTemp;
    unsigned int libOsInfoVersionTemp;

    this->loadFromResources(m_osInfoData, m_libOsInfoVersion);
    this->loadFromAppConfigDir(osInfoDataTemp, libOsInfoVersionTemp);

    // *** Если файл ~/.config/libvirt-snapshot-manager/libOsInfo.json *** //
    //     не найден, то в libOsInfoVersionTemp возвращается ноль,         //
    //     что всегда меньше  версии встроенных в приложение данных.       //
    if (libOsInfoVersionTemp > m_libOsInfoVersion){
        m_osInfoData = osInfoDataTemp;
        m_libOsInfoVersion = libOsInfoVersionTemp;
    }
}

OsInfo OsInfoProvider::getOsInfoByOsId(const QString& osId){
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

    // *** Обработка случая, когда id в базе libosinfo-db не найден, *** //
    //       возвращение искомого id c пустыми остальными полями.        //
    if (res.id.isEmpty()){
        res.id = osId;
        res.name = "";
        res.ram = "";
        res.vendor = "";
    }

    return res;
}

void OsInfoProvider::loadFromResources(QVector<OsInfo>& osInfoData,
                                                unsigned int& libOsInfoVersion){
    QString jsonAppResFile = ":/libOsInfo.json";
    this->loadFromJson(jsonAppResFile, osInfoData, libOsInfoVersion);
}

void OsInfoProvider::loadFromAppConfigDir(QVector<OsInfo>& osInfoData,
                                                unsigned int& libOsInfoVersion){
    // *** Значение по умолчанию *** //
    libOsInfoVersion = 0;

    QString localLibOsInfoData = getLocalLibOsInfoFullFileName();
    this->loadFromJson(localLibOsInfoData, osInfoData, libOsInfoVersion);
}

void OsInfoProvider::loadFromJson(const QString& fileName,
                   QVector<OsInfo>& osInfoData, unsigned int& libOsInfoVersion){
    QFile file(fileName);
    osInfoData.clear();

    if (!file.open(QIODevice::ReadOnly)){
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()){
        qDebug() << "[EE] Invalid JSON";
        return;
    }

    QJsonArray osInfoJsonArray = doc.object()["osInfo"].toArray();

    for (int i = 0; i < osInfoJsonArray.size(); ++i){
        QJsonObject obj = osInfoJsonArray[i].toObject();
        OsInfo os;

        os.id = obj["id"].toString();
        os.name = obj["name"].toString();
        os.vendor = obj["vendor"].toString();
        os.ram = obj["ram"].toString();

        osInfoData.push_back(os);
    }

    libOsInfoVersion = doc.object()["libOsInfoVersion"].toString().toInt();
}

unsigned int OsInfoProvider::getLocalLibOsInfoVersion(){
    if (m_osInfoData.isEmpty()){
        this->dataInit();
    }

    return m_libOsInfoVersion;
}

unsigned int OsInfoProvider::getLatestLibOsInfoVersion(){
    unsigned int res;

    res = m_osInfoLoader->getLatestLibOsInfoVersion();
    return res;
}

void OsInfoProvider::updateLocalLibOsInfo(){
    QJsonDocument newLibOsInfoJsonDoc;
    newLibOsInfoJsonDoc = m_osInfoLoader->getLibOsInfoJson();
    if (newLibOsInfoJsonDoc.isEmpty()){
        return;
    }

    // *** Перестраховка с отменой сохранения файлов прежней версии *** //
    int newVersion = newLibOsInfoJsonDoc.object()["libOsInfoVersion"]
                                                            .toString().toInt();
    if (newVersion > m_libOsInfoVersion){
        QString localLibOsInfoFileFullName = getLocalLibOsInfoFullFileName();
        backupLocalLibOsInfo(localLibOsInfoFileFullName);
        m_osInfoLoader->writeLibOsInfoJsonToFile(newLibOsInfoJsonDoc,
                                                    localLibOsInfoFileFullName);
    }

    // *** Загрузка новых даных из сохранёного json-файла *** //
    this->dataInit();

    // *** Отправка сигнала о том, что база данных изменилась *** //
    //     и теперь можно обновить данные.                        //
    emit localOsInfoUpdateFinished();
}


void OsInfoProvider::backupLocalLibOsInfo(const QString& srcFileFullName){
    QString dstFileFullName = srcFileFullName + ".bak";

    if (!QFile::exists(srcFileFullName)){
        return;
    }

    if (QFile::exists(dstFileFullName)){
        if (!QFile::remove(dstFileFullName)){
            return;
        }
    }

    QFile::rename(srcFileFullName, dstFileFullName);
}

QString OsInfoProvider::getLocalLibOsInfoFullFileName(){
    QString userAppConfigDir;
    QStandardPaths::StandardLocation dirType = QStandardPaths::ConfigLocation;
    userAppConfigDir = QStandardPaths::writableLocation(dirType);
    QString localLibOsInfoFileFullName = userAppConfigDir
                        + "/" + m_appConfigDirName + "/" + m_localLibOsInfo;
    return localLibOsInfoFileFullName;
}

void OsInfoProvider::reloadData(){
    this->dataInit();
}

QStringList OsInfoProvider::getOsNameList(){
    QStringList osNameList;

    if (m_osInfoData.isEmpty()){
        this->dataInit();
    }

    for (int i = 0; i < m_osInfoData.size(); ++i){
        osNameList.push_back(m_osInfoData[i].name);
    }

    return osNameList;
}

OsInfo OsInfoProvider::getOsInfoByOsName(const QString& osNameIn){
    if (m_osInfoData.isEmpty()){
        this->dataInit();
    }

    OsInfo res;
    QString osName = osNameIn;
    osName = osName.simplified();

    for (int i = 0; i < m_osInfoData.size(); ++i){
        if (osName == m_osInfoData[i].name){
            res = m_osInfoData[i];
            break;
        }
    }

    return res;
}
// End osInfoProvider.cpp
