// Begin snapManager.cpp

#include "snapManager.h"

SnapManager::SnapManager(QWidget* parent) : parentWindow(parent){
}

SnapManager::~SnapManager(){
}

void SnapManager::doSnapshot(const QString& name, const QStringList& mntDisks){

    for (int i = 0; i < mntDisks.size(); ++i){
        qDebug() << "[II] VM mount storage:" << mntDisks[i];
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
                    parentWindow, "Take Snapshot - " + name,
                        "Do you really want to take a snapshot of '"+ name +"'",
                                            QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    qDebug() << "[II] Take snapshot " + name;

    // parentId по имени дного из примонтированных дисков
    // id - число секунд с 1970-ого года
    QString parentId = getStorageId(mntDisks[0]);
    QString id = QString::number(QDateTime::currentSecsSinceEpoch());

    // Создание массива имён снапшотов
    QStringList snapshotsFullNames;
    for (int i = 0; i < mntDisks.size(); ++i){
        snapshotsFullNames.push_back(getSnapName(mntDisks[i], id, parentId));
    }

    // // Создание и запуск внешней команды
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");

    QProcess process;
    process.setProcessEnvironment(env);

    // Индексы у mntDisks и snapshotsFullNames согласованы
    for (int n = 0; n < snapshotsFullNames.size(); ++n){
        QString snapName = snapshotsFullNames[n];
        QStringList qemuImgArguments = {
                                            "create",
                                            "-f",
                                            "qcow2",
                                            "-b", mntDisks[n],
                                            "-F",
                                            "qcow2",
                                            snapshotsFullNames[n]
                                        };
        process.start("qemu-img", qemuImgArguments);

        if (!process.waitForStarted()){
            return;
        }

        if (!process.waitForFinished()){
            return;
        }
    }

    // Смена точки монтирования в VM
    switchVmMountStorages(name, snapshotsFullNames);
}

QString SnapManager::getStorageId(const QString& imgName){
    QString imageId;

    // Определение id из имени imgName
    // "0123456789-at-0123456789"
    QRegularExpression pattern1(R"((\d{10}-at-\d{10}))");
    QRegularExpressionMatch match1 = pattern1.match(imgName);

    if (!match1.hasMatch()){
        imageId = "0000000000";
    } else {
        QString subString = match1.captured(1);
        // 0123456789
        QRegularExpression pattern2(R"((\d{10}))");
        QRegularExpressionMatch match2 = pattern2.match(subString);

        if (match2.hasMatch()) {
            imageId = match2.captured(1);
        }
    }
    qDebug() << imageId;
    return imageId;
}

QString SnapManager::getSnapName(const QString& imgName, const QString& id,
                                                       const QString& parentId){
    QString snapName = imgName;

    if (snapName.endsWith(".qcow2", Qt::CaseInsensitive)) {
        snapName.chop(6);
    }

    QRegularExpression pattern1(R"((-\d{10}-at-\d{10}))");
    snapName = snapName.replace(pattern1, "");
    snapName += "-" + id + "-at-" + parentId + ".qcow2";
    return snapName;
}

void SnapManager::switchVmMountStorages(const QString& vmName,
                                               const QStringList& snapStorages){
    // *** Получение vmXmlDoc *** //
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");

    QProcess process;
    process.setProcessEnvironment(env);
    process.start("virsh", {"dumpxml", vmName});

    if (!process.waitForStarted()){
        return;
    }

    if (!process.waitForFinished()){
        return;
    }

    QString output = process.readAllStandardOutput();

    QDomDocument vmXmlDoc;
    vmXmlDoc.setContent(output);
    QDomElement vmXml = vmXmlDoc.documentElement();

    // *** Получение вектора элементов <source> *** //
    QVector<QDomElement> sourcesList;
    QDomElement devices = vmXml.firstChildElement("devices");
    QDomNodeList diskNodes  = devices.elementsByTagName("disk");
    for (int i = 0; i < diskNodes.count(); ++i){
        QDomElement diskNode = diskNodes.item(i).toElement();
        if ((diskNode.attribute("type") == "file")
                                    && (diskNode.attribute("device") == "disk"))
        {
            if (diskNode.firstChildElement("driver").attribute("type")=="qcow2")
            {
                QDomElement source = diskNode.firstChildElement("source");
                sourcesList.push_back(source);
            }
        }
    }

    // *** изменение файлов подключенных дисков на созданные *** //
    for (int i = 0; i < sourcesList.size(); ++i){
        sourcesList[i].setAttribute("file", snapStorages[i]);
    }

    // *** Сохранение временного xml файла c обновлёнными настройками *** //
    QString fileName = "/tmp/" + vmName + ".xml";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text
                                                       | QIODevice::Truncate)){
        qWarning() << "[EE] Failed to write back XML file.";
        return;
    }

    QTextStream out(&file);
    vmXmlDoc.save(out, 4);
    file.close();

    // *** Применение новых настроек через virsh *** //
    process.start("virsh", {"define", fileName});

    if (!process.waitForStarted()){
        return;
    }

    if (!process.waitForFinished()){
       return;
    }

    // *** Удаление временного xml файла *** //
    if (QFile::exists(fileName)) {
        if (!QFile::remove(fileName)) {
            qDebug() << "[EE] don't delete:" << fileName;
        }
    }
}
// End snapManager.cpp
