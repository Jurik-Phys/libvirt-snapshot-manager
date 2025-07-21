// Begin snapManager.cpp

#include "snapManager.h"

SnapManager::SnapManager(QObject*){
}

SnapManager::SnapManager(const VMachine& vm, QObject*){
    m_vm = vm;
}


SnapManager::~SnapManager(){
}


QDomElement SnapManager::getVmXml(const QString& vm){
    QDomElement res;

    QProcess process;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");
    process.setProcessEnvironment(env);

    QString cmd = QString("virsh dumpxml ") + vm;
    QString     virshCMD = cmd.split(" ")[0];
    QStringList virshARG = { cmd.split(" ")[1], cmd.split(" ")[2] };

    process.start(virshCMD, virshARG);

    if (!process.waitForStarted())
        return res;

    if (!process.waitForFinished())
        return res;

    QString output = process.readAllStandardOutput();

    QDomDocument xmlDoc;
    xmlDoc.setContent(output);

    res = xmlDoc.documentElement();

    return res;
}

QStringList SnapManager::getFullPathDisks(const VMachine& vm){
    QStringList res;

    QDomElement vmXml = getVmXml(vm.name);

    QDomNodeList disks = vmXml.elementsByTagName("disk");

    for (int i = 0; i < disks.count(); ++i) {
        QDomElement disk = disks.at(i).toElement();

        QString diskType = disk.attribute("type");
        QString drvType  = disk.firstChildElement("driver").attribute("type");

        if ( diskType == "file" && drvType == "qcow2" ) {
            QString path = disk.firstChildElement("source").attribute("file");
            res.push_back(path);
        }
    }

    return res;
}

QStringList SnapManager::getBaseDirsList(const QStringList& disksList){

   /*
    *  Снапшоты должны лежать в том же каталоге, что и снапшотируемый диск.
    *  В противном случае, новый снапшот будет располагаться вновом каталоге
    *  и для системы он ничем не будет отличаться от диска базового, а значит
    *  изменится baseDirs и новый снапшот будет уходить вглубь, создавая хаос
    *  из вложенных снапшотов.
    *
    */

    QStringList res;
    for (int i = 0; i < disksList.size(); ++i){
        QString dir = QFileInfo(disksList[i]).absolutePath();
        res.push_back(dir);
    }

    return res;
}

QStringList SnapManager::getFileNameDisks(const QStringList& disksList){
    QStringList res;

    for (int i = 0; i < disksList.size(); ++i){
        QString dir = QFileInfo(disksList[i]).fileName();
        res.push_back(dir);
    }

    return res;
}

// TODO DELETE? //
QString SnapManager::getCoreFileNames(const QStringList& filesList){
    QString res;
    QStringList names = filesList;

   /*
    * coreFileName - это часть имени файла без расширения ".qcow2",
    * без префиксов образованных снапшотированием "xxx-snap", "xxx-work",
    * "xxx-work-xxx"
    *
    * Побуквенный сбор строки при совпадении символов во всех строках.
    *
    */

    for (int i = 0; i < names.size(); ++i) {
        if (names[i].endsWith(".qcow2", Qt::CaseInsensitive)) {
            names[i].chop(6);
        }
    }

    return res;
}

QString SnapManager::getBackingFile(const QString& fullFileName){

    QString filesBaseDir = QFileInfo(fullFileName).absolutePath();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");

    QProcess process;
    process.setProcessEnvironment(env);
    process.start("qemu-img", {"info", fullFileName});

    if (!process.waitForFinished())
        return fullFileName;

    QString output = process.readAllStandardOutput();

    QString backingFile;
    if (output.contains("backing file")){
        QStringList outputLines = output.split('\n', Qt::SkipEmptyParts);
        for (int i = 0; i < outputLines.size(); ++i){
            QString line = outputLines[i];
            if (line.split(":", Qt::SkipEmptyParts)[0] == "backing file"){
                backingFile = line.split(":", Qt::SkipEmptyParts)[1];
                // Надпись "(actual path ... )" у коротких названий есть,
                // а у длинных отсутствует и путь выдаётся полный,
                // поэтому испольуется "хак", чтобы оба способа работали
                backingFile = backingFile.split("(", Qt::SkipEmptyParts)[0];
                backingFile = backingFile.trimmed();
                backingFile = QFileInfo(backingFile).fileName();
                backingFile = filesBaseDir + "/" + backingFile;
                break;
            }
        }
    }
    else {
        // No backing file => file is root of disk chains
        backingFile = "";
    }

    return backingFile;
}

QString SnapManager::getRootDisksChain(const QString& inFile){

    QString preBackingFile;
    QString backingFile = inFile;

    while (backingFile != ""){
        if (backingFile != "" ){
            preBackingFile = backingFile;
            backingFile = getBackingFile(backingFile);
        }
        else {
            break;
        }
    }

    return preBackingFile;
}


void SnapManager::takeSnapshot(const VMachine& vm){
    qDebug() << "[II] Take snapshot now for:" << vm.name;

    QStringList disksList = getFullPathDisks(vm);

    qDebug() << "[II] VM disks:";
    for (int i = 0; i < disksList.size(); ++i){
        qDebug() << QString::number(i+1) + ". " + disksList[i];
    }

    QStringList baseDirs  = getBaseDirsList(disksList);
    QStringList fileNames = getFileNameDisks(disksList);
    // QString coreFielName  = getCoreFileNames(fileNames);


    qDebug() << "[II] VM base dirs & files:";
    for (int i = 0; i < baseDirs.size(); ++i){
        qDebug() << QString::number(i+1) + ". " + baseDirs[i];
        qDebug() << QString::number(i+1) + ". " + fileNames[i];
    }

    // QString file = "/home/jurik_phys/.kvm/win/WinXP/Windows.XP.qcow2";
    QString file = "/home/jurik_phys/.kvm/win/WinXP/Windows.XP.1752476739-snap.qcow2";
    // QString file = "/home/jurik_phys/.kvm/win/WinXP/Windows.XP.snapshot.snapshot.state-1";

    // QString file = "/home/jurik_phys/.kvm/win/WinXP/Windows.XP.1752433579-work-1752476211.qcow2";

    QString rootDisksChain = getRootDisksChain(file);
    qDebug() << "[II] Root of chain:" << rootDisksChain;
}

QVector<ChainNode> SnapManager::getSnapTreeModelData(const VMachine& vm){
    QVector<ChainNode> res;
    QStringList disksList = getFullPathDisks(vm);

    // На данном этапе работа с одним диском (текущий диск)
    QString disk = disksList[0];
    QString dir  = QFileInfo(disk).absolutePath();
    QString snapRoot = getRootDisksChain(disk);

    // Получение всех файлов в каталоге с текущим диском
    QStringList allFiles = QDir(dir).entryList(QDir::Files | QDir::NoSymLinks |
                                                      QDir::Hidden, QDir::Name);

    // Получение файлов, принадлежащих той же цепочке, что и текущий диск.
    QStringList onlyDiskChainFiles;
    for (int i = 0; i < allFiles.size(); ++i){
        QString rootDisk;
        QString fullPathFileI = dir + "/" + allFiles[i];
        rootDisk = getRootDisksChain(fullPathFileI);
        if ((rootDisk == snapRoot) && (fullPathFileI != snapRoot)) {
            onlyDiskChainFiles.push_back(allFiles[i]);
        }
    }

    // Добавление корневого узла в вектор данных
    res.push_back({1, -1, snapRoot, "This is Root"});

   /*
    * Ищем всех потомков для первого элемента вектора данных.
    * Каждого найденного потомка добавляем в общий список, удаляя
    * из onlyDiskChainFiles. После того, как все потомки для первого элемента
    * найдены (т.е., больше не находятся те, у кого backing-file - это первый
    * элемент). Второй элемент становится родителем, поиск потомков повторяется.
    *
    */

    int parentId = 1;
    while(!onlyDiskChainFiles.isEmpty()){
        QString parent = res[parentId - 1].name;
        QString fileName, fullFileName;
        QStringList solution;
        // Проход по всему списку и сбор потомков для parentId
        for (int i = 0; i < onlyDiskChainFiles.size(); ++i){
            fileName = onlyDiskChainFiles[i];
            fullFileName = dir + "/" + fileName;
            QString backingFile = getBackingFile(fullFileName);
            if ( parent == backingFile ){
                solution.push_back(fullFileName);
            }
        }
        // Удаление найденных потомков и занесение их в вектор данных
        for (int i = 0; i < solution.size(); ++i){
            onlyDiskChainFiles.removeOne(QFileInfo(solution[i]).fileName());
            res.push_back({(int)res.size() + 1, parentId, solution[i],
                  "This is data with ID: " + QString::number(res.size() + 1) });
        }
        parentId++;
    }

    return res;
}

void SnapManager::setVmName(const VMachine& vm){
    m_vm = vm;
}

void SnapManager::process(){
    QVector<ChainNode> result;
    result = getSnapTreeModelData(m_vm);
    emit finished(result);
}

// End snapManager.cpp
