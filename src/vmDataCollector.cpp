// Begin vmDataCollector.cpp

#include "vmDataCollector.h"

VmDataCollector::VmDataCollector(QWidget* parent) : parentWindow(parent){
    m_getListTimer = new QTimer(this);
    QObject::connect(m_getListTimer, &QTimer::timeout,
                                          this, &VmDataCollector::vmListSender);
}

VmDataCollector::VmDataCollector(const VMachine& vm, QWidget* parent) {
    parentWindow = parent;
    m_vm = vm;
}

VmDataCollector::~VmDataCollector(){
}

// Вектор из {name, state}
QVector<VMachine> VmDataCollector::getVmList(){
    QVector<VMachine> vmList;
    QVector<VMachine> uuidVmList;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");

    QProcess process;
    process.setProcessEnvironment(env);

    QEventLoop loop;

    bool stepOneDone = false;
    QObject::connect(&process, &QProcess::finished,
        [&](int, QProcess::ExitStatus){
            // Разбор вывода в finished, а не в readyReadStandardOutput т.к.,
            // вторая команда для получения uuid выдаёт информацию порциями,
            // что может привести к пропускам в данных, если не использовать
            // буфер для наколпения вывода. Данный способ проще в реализации.
            QString output = process.readAllStandardOutput();
            if (!stepOneDone){
                stepOneDone = true;

                // Разбиваем на строки и сохраняем реузльтат первой команды
                QStringList outputLines = output.split('\n',Qt::SkipEmptyParts);
                for (int i = 0; i < outputLines.size(); ++i){
                    // Skip table header
                    if (i > 1){
                        VMachine vm;
                        QString line = outputLines[i].trimmed()
                                   .replace(QRegularExpression("\\s{2,}"), " ");
                        QStringList parts = line.split(" ");
                        if (parts.size() >=3){
                            vm.name  = parts[1];
                            vm.state = parts.mid(2).join(" ");
                        }
                        vmList.append(vm);
                    }
                }

                // Запуск второго этапа получени uuid виртуальных машин
                process.start("virsh", {"list", "--all", "--uuid", "--name"});
            }
            else{
                // *** Обработка всего стандартного вывода второй команды *** //
                QStringList outputLines = output.split('\n',Qt::SkipEmptyParts);
                for (int i = 0; i < outputLines.size(); ++i){
                    QStringList line = outputLines[i].split(" ");
                    VMachine vm;
                    vm.name = line[1];
                    vm.uuid = line[0];
                    uuidVmList.append(vm);
                }

                // *** Слияние информации об uuid виртуальных машин *** //
                for (int i = 0; i < vmList.size(); ++i){
                    // *** Очерёдность виртуальных машин не гарантирована *** //
                    for (int j = 0; j < vmList.size(); ++j){
                        if (vmList[i].name == uuidVmList[j].name){
                            vmList[i].uuid = uuidVmList[j].uuid;
                        }
                    }
                }
                loop.quit();
            }
        });

    process.start("virsh", {"list", "--all"});
    loop.exec();
    return vmList;
}

VMachine VmDataCollector::getVmInfo(const VMachine& vmIn){
    VMachine vm;
    vm.name = vmIn.name;
    vm.state = vmIn.state;

    // Virtual machine information in XML
    QDomDocument vmXmlDoc = getVmXml(vm.name);

    QDomElement vmXml = vmXmlDoc.documentElement();
    vm.uuid = vmXml.firstChildElement("uuid").text();
    vm.ram = vmXml.firstChildElement("memory").text()
                    + " " + vmXml.firstChildElement("memory").attribute("unit");
    vm.osId = vmXml.firstChildElement("metadata")
                                    .firstChildElement("libosinfo:libosinfo")
                                        .firstChildElement("libosinfo:os")
                                            .attribute("id");
    vm.cpu = "Virtual CPUs: " + vmXml.firstChildElement("vcpu").text() + " (";

    QDomNamedNodeMap cpuAttribues = vmXml.firstChildElement("cpu")
                                    .firstChildElement("topology").attributes();
    for (int i = 0; i < cpuAttribues.count(); ++i ){
        QDomNode attr = cpuAttribues.item(i);
        QString name = attr.nodeName();
        QString value = attr.nodeValue();
        vm.cpu = vm.cpu + name + " " + value + "; ";
    }
    vm.cpu.chop(2);
    vm.cpu = vm.cpu + ")";

    QDomElement devices = vmXml.firstChildElement("devices");
    QDomNodeList diskNodes  = devices.elementsByTagName("disk");
    for (int i = 0; i < diskNodes.count(); ++i){
        QDomElement diskNode = diskNodes.item(i).toElement();
        if ((diskNode.attribute("type") == "file")
                                    && (diskNode.attribute("device") == "disk"))
        {
            if (diskNode.firstChildElement("driver").attribute("type")=="qcow2")
            {
                QString mountStorage = diskNode.firstChildElement("source")
                                                            .attribute("file");
                // Точка монтирования может являться символической ссылкой,
                // необходимо это проверить и разрешить путь при необходимости
                QFileInfo file(mountStorage);
                if (!file.exists()){
                    qDebug() << "[II] The virtual machine storage isn't "
                                                   "available:" << mountStorage;
                    emit errorMsg("File access error …",
                        "The virtual machine storage is not available:\n"
                        + QString(" - Base path: ") + QFileInfo(mountStorage)
                                                            .absolutePath()+"\n"
                        + QString(" - File name: ") + QFileInfo(mountStorage)
                                                                   .fileName());
                    return vm;
                }

                if (file.isSymLink()){
                    mountStorage = QFileInfo(mountStorage).absolutePath() +"/"+
                                 QFileInfo(file.canonicalFilePath()).fileName();
                }
                vm.mountStorages.push_back(mountStorage);

                // Установка каталога цепочки сохранения состояния
                QString snapshotsDir = QFileInfo(mountStorage).absolutePath();
                vm.snapshotsDirs.push_back(snapshotsDir);
            }
        }
    }

    // В одном каталоге может быть несколько цепочек сохранения состояний,
    // читать дважды информацию о файлах для каждой цепочки сохранения не надо
    // Оставляем только уникальные каталоги цепочек сохранения
    QSet<QString> uniqueSet;
    for (int i = 0; i < vm.snapshotsDirs.size(); ++i){
        uniqueSet.insert(vm.snapshotsDirs[i]);
    }
    vm.snapshotsDirs = uniqueSet.values();

    qDebug() << "\n[II] VM.INFO:            ";
    qDebug() <<   "[II]    > Name:          " << vm.name;
    qDebug() <<   "[II]    > UUID:          " << vm.uuid;
    qDebug() <<   "[II]    > OsID:          " << vm.osId;
    qDebug() <<   "[II]    > State:         " << vm.state;
    qDebug() <<   "[II]    > CPU:           " << vm.cpu;
    qDebug() <<   "[II]    > RAM:           " << vm.ram;
    // Вывод информации о подключеных к ВМ дисках
    for (int i = 0; i < vm.mountStorages.size(); ++i){
        if (i == 0){
            qDebug() << "[II]    > Mount storages:"
                   << QString::number(i + 1) + ". " + vm.mountStorages[i];
        }
        else {
            qDebug() << "[II]                     "
                   << QString::number(i + 1) + ". " + vm.mountStorages[i];
        }
    }
    // Вывод информаци о каталогах со снапшотами
    for (int i = 0; i < vm.snapshotsDirs.size(); ++i){
        if (i == 0){
            qDebug() << "[II]    > Snapshots dirs:"
                   << QString::number(i + 1) + ". " + vm.snapshotsDirs[i];
        }
        else {
            qDebug() << "[II]                     "
                   << QString::number(i + 1) + ". " + vm.snapshotsDirs[i];
        }
    }

    // Загрузка данных о qcow2 файлах, из каталогов сохранения цепочек состояний
    m_vmImagesRawInfo.clear();
    for ( int i = 0; i < vm.snapshotsDirs.size(); ++i ){
        loadVmImagesRawInfoOverQEMU(vm.snapshotsDirs[i]);
    }

    // Определение корневых файлов для каждой цепочки сохранения
    for (int i = 0; i < vm.mountStorages.size(); ++i){
        vm.rootFullName.push_back(getRootFullName(vm.mountStorages[i]));

        // Вывод информаци о корневых файлах каждой из цепочек сохранения
        if (i == 0){
            qDebug() << "[II]    > Storage's root:"
                            << QString::number(i+1) +". " + vm.rootFullName[i];
        }
        else {
            qDebug() << "[II]                     "
                            << QString::number(i+1) +". " + vm.rootFullName[i];
        }
    }

    // Построение цепочек сохранённых состояний
    setSnapChainData(vm);

    // Заполнение данных о потомках каждого узла
    // (необходимо для реализации удаления узла внутри цепочки)
    setChildrenData(vm);

    return vm;
}

QDomDocument VmDataCollector::getVmXml(const QString& vmName){
    QDomDocument vmXmlDoc;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");

    QProcess process;
    process.setProcessEnvironment(env);
    process.start("virsh", {"dumpxml", vmName});

    if (!process.waitForStarted()){
        return vmXmlDoc;
    }

    if (!process.waitForFinished()){
        return vmXmlDoc;
    }

    QString output = process.readAllStandardOutput();
    vmXmlDoc.setContent(output);

    return vmXmlDoc;
}

void VmDataCollector::setSnapChainData(VMachine& vm){

    // Данные для корневой точки;
    int id = 1;
    int parentId = -1;
    while (true){
        ChainNode node;
        node.imagesType = "work";
        for (int mntIdx = 0; mntIdx  < vm.mountStorages.size(); ++mntIdx){
            // Формирование корневого узла в цепочке сохранений;
            if (parentId == -1){
                node.id = id;
                node.parentId = parentId;
                node.imagesType = "active";
                // Если у диска есть потомки, то он не может быть изменён
                // т.е., выполняет роль хранителя состояния, снимка,
                // его тип "snap". По умолчанию тип состояния "work"
                QString imageFullName = vm.rootFullName[mntIdx];
                QString imageFileName = QFileInfo(imageFullName).fileName();
                // Есть ли у корневого диска потомки?
                for (int i = 0; i < m_vmImagesRawInfo.size(); ++i){
                    if (imageFullName == m_vmImagesRawInfo[i].backFullName){
                        node.imagesType = "snap";
                        break;
                    }
                }
                node.imagesFullNames.push_back(imageFullName);
                node.backFullNames.push_back("None");

                // Имя узла задано на основе хеша + списка словосочетаний
                node.name = getNodeName(imageFileName, node.imagesType);
            }
            // Формирование всех остальных узлов в цепочке сохранений
            else {
                // Алгоритм:
                // 1. Фиксируется родитель (parentId) и идёт поиск его потомков.
                //    При нахождении информация о найденных потомках добавляется
                //    в узел, а из общего хранилища потомки удаляются.
                //    Далее повтор поиска иных потомков для того же родителя.
                // 2. Если потомки не найдены для (parentId), то начинается
                //    поиск потомков для следующего родителя (parentId + 1);
                // 3. Поиск прекращается, когда parentId будет больше id
                //    последнего рассчитываемого узла.

                id = vm.vmStateChain.size() + 1;
                QString parentImage = vm.vmStateChain[parentId - 1]
                                                       .imagesFullNames[mntIdx];
                for (int i = 0; i < m_vmImagesRawInfo.size(); ++i){
                    QString& backFullName = m_vmImagesRawInfo[i].backFullName;
                    QString& imageFullName = m_vmImagesRawInfo[i].imageFullName;
                    QString imageFileName = QFileInfo(imageFullName).fileName();

                    if (parentImage == backFullName ){
                        // Фиксируем некоторые параметры узла цепочки снапшотов
                        node.id = id;
                        node.parentId = parentId;

                        // Определение типа узла (work|snap)
                        // Если у диска есть потомки, то узел "snap"
                        for (int n = 0; n < m_vmImagesRawInfo.size(); ++n){
                            if (imageFullName
                                          == m_vmImagesRawInfo[n].backFullName){
                                node.imagesType = "snap";
                                break;
                            }
                        }

                        // Имя узла по его хешу и списку словосочетаний
                        node.name = getNodeName(imageFileName, node.imagesType);

                        // Если точка без потомков (work) и совпадает с точкой
                        // монтирования к ВМ, то это активная рабочая точка
                        if (node.imagesType == "work"){
                            // Потомков нет и текущий диск примонтирован к ВМ
                            if (imageFullName == vm.mountStorages[mntIdx] ){
                                node.imagesType = "active";
                            }
                        }

                        // Фиксация найденных решений
                        node.imagesFullNames.push_back(imageFullName);
                        node.backFullNames.push_back(backFullName);

                        // Удаление найденного вхождения одного из mountStorage
                        // и переход к проверке следующего mountStorage
                        m_vmImagesRawInfo.removeAt(i);
                        break;
                    }
                }
               }
        }
        // Новый узел не сформирован, смена родителя
        if (node.imagesFullNames.size() == 0){
            parentId++;
            // Если после увеличения id родителя, его значение превышает число
            // элементов в цепочке состояний, то выходим из построения цепочки.
            if (parentId > vm.vmStateChain.size()){
                return;
            }
        } else {
            vm.vmStateChain.push_back(node);
            // qDebug() << "Добавление узла:";
            // qDebug() << "             id:" << id;
            // qDebug() << "       parentId:" << parentId;
        }

        // Change parentId after first node
        if (parentId == -1){
            parentId = 1;
        }
    }
}

void VmDataCollector::setChildrenData(VMachine &vm){
    // Установление childrenId
    for (int i = 0; i < vm.vmStateChain.size(); ++i){
        QStringList childrenFullNames;
        // Критерий "потомства": parentId узла указывает на id текущего узла.
        // Узел с таким свойством - потомок, сохраняем список imagesFullNames
        // таких списков для всех потомков
        for (int j = i + 1; j < vm.vmStateChain.size(); ++j){
            if (vm.vmStateChain[i].id == vm.vmStateChain[j].parentId){
                childrenFullNames = vm.vmStateChain[j].imagesFullNames;
                vm.vmStateChain[i].childrenImagesFullNames
                                                 .push_back(childrenFullNames);
            }
        }
    }
}

bool VmDataCollector::isVMachineImage(const QString& imageFullName){
    // Определение принадлежности файла к qcow2 по "Magic number"
    // https://www.ijrte.org/wp-content/uploads/papers/v8i5/E5606018520.pdf
    // 514649FB - магическое число qcow2 файлов (см. табл. 2)
    bool vmImage = false;

    QFile file(imageFullName);

    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "[EE] Error access" << imageFullName;
        emit errorMsg("File access error …",
                     "Please check your access to the file:\n" + imageFullName);
        return false;
    }

    QByteArray header = file.read(4);

    if (header == QByteArray::fromHex("514649FB")){
       vmImage = true;
    }

    return vmImage;
}

void VmDataCollector::loadVmImagesRawInfoOverQEMU(const QString& snapshotsDir){
    QVector<VmImageRawInfo> vmImagesRawInfo;

    // Получение списка всех файлов из каталога цепочки сохранения состояний
    // /* только имена файлов */
    QStringList basePathFiles = QDir(snapshotsDir).entryList(QDir::Files
                                | QDir::NoSymLinks | QDir::Hidden, QDir::Name);

    // qDebug() << "\n[II] Каталог цепочки сохранения:" << snapshotsDir;

    // Проверка на доступность (файлы всегда должны быть по логике программы)
    if (basePathFiles.size() == 0){
        qDebug() << "[EE] Error open directory:" << snapshotsDir;
        emit errorMsg("Directory access error …",
                 "Please check your access to the directory:\n" + snapshotsDir);
        return;
    }

    // В 'snapshotsDir' символические ссылки отбрасыаются:
    // - симмволическая ссылка может вести в другой каталог, что вызовет "кашу"
    //   снапшотов;
    // - если символическая ссылка ведёт на файл в текущем каталоге,
    //   то возникнет очевидный дубль.
    // Символические ссылки необходимо учитывать при определении 'mountStorage'

    // Загружаем информацию о qcow2 файлах из каталога цепочки сохранения
    for (int i = 0, vmDiskCount = -1; i < basePathFiles.size(); ++i){
        QString fileFullName = snapshotsDir + "/" + basePathFiles[i];
        if (isVMachineImage(fileFullName)){
            vmDiskCount++;
            VmImageRawInfo vmImageRawInfo;
            vmImageRawInfo.imageBasePath = snapshotsDir;
            vmImageRawInfo.imageFullName = fileFullName;
            vmImageRawInfo.backFullName = getBackFullNameQEMU(vmImageRawInfo);
            // backFullName может быть символической ссылкой на реальный файл.
            // необходимо это проверить и разрешить путь при необходиомсти.
            // Предполагается, что реальный файл находится в этом же каталоге
            QFileInfo file(vmImageRawInfo.backFullName);
            if (file.isSymLink()){
                vmImageRawInfo.backFullName = snapshotsDir +"/"+
                                 QFileInfo(file.canonicalFilePath()).fileName();
            }
            // Проверка на существование backFullName,
            // "-1" исключается из проверки т.к.,
            // соответствует штатному отсутствию backing-file (корневой диск)
            if (!QFileInfo(vmImageRawInfo.backFullName).isFile() &&
                                           vmImageRawInfo.backFullName != "-1"){
                qDebug() << "[EE] Error open backing-file:" <<
                                                    vmImageRawInfo.backFullName;
                QMessageBox::critical(parentWindow, "Ошибка доступа к файлу",
                      "Проверьте backing файл:\n" +vmImageRawInfo.backFullName);
                vmImageRawInfo.backFullName = "[Not found] "
                                                  + vmImageRawInfo.backFullName;
                return;
            }

            vmImageRawInfo.inChain = false;
            m_vmImagesRawInfo.push_back(vmImageRawInfo);

            // qDebug() << "        > File Index ="  << vmDiskCount;
            // qDebug() << "    [i] >" << vmImageRawInfo.imageFullName;
            // qDebug() << "    [b] >" << vmImageRawInfo.backFullName;
        }
    }
}

QString VmDataCollector::getBackFullNameQEMU(const VmImageRawInfo& vmImgRawInf){
    QString backingFile;

    QString baseDir = QFileInfo(vmImgRawInf.imageFullName).absolutePath();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");

    QProcess process;
    process.setProcessEnvironment(env);
    QEventLoop loop;
    QObject::connect(&process, &QProcess::finished, &loop, &QEventLoop::quit);

    QObject::connect(&process, &QProcess::readyReadStandardOutput,
        [&](){
            QString output = process.readAllStandardOutput();

            if (output.contains("backing file")){
                QStringList outLines = output.split('\n', Qt::SkipEmptyParts);
                for (int i = 0; i < outLines.size(); ++i){
                    QString ln = outLines[i];
                    if (ln.split(":", Qt::SkipEmptyParts)[0] == "backing file"){
                        backingFile = ln.split(":", Qt::SkipEmptyParts)[1];
                        // Надпись "(actual path ... )" у коротких названий есть,
                        // а у длинных отсутствует и путь выдаётся полный,
                        // поэтому испольуется "хак", чтобы оба способа работали
                        backingFile = backingFile
                                             .split("(", Qt::SkipEmptyParts)[0];
                        backingFile = backingFile.trimmed();
                        backingFile = QFileInfo(backingFile).fileName();
                        backingFile = baseDir + "/" + backingFile;
                        break;
                    }
                }
            }
            else {
                // No backing file => file is root of disk chains
                backingFile = "-1";
            }
        });

    process.start("qemu-img", {"info", "-U", vmImgRawInf.imageFullName});
    loop.exec();

    return backingFile;
}

QString VmDataCollector::getBackFullNameFast(const QString& imageFullName){
    QString imageBasePath = QFileInfo(imageFullName).absolutePath();
    QString backFullName;

    for (int i = 0; i < m_vmImagesRawInfo.size(); ++i ){
        // Проверка совпадения по каталогу
        if ( imageBasePath == m_vmImagesRawInfo[i].imageBasePath ){
            // Совпадение imageFullName's, фиксация результата и выход из цикла
            if (m_vmImagesRawInfo[i].imageFullName == imageFullName){
                backFullName = m_vmImagesRawInfo[i].backFullName;
                break;
            }
        }
    }

    return backFullName;
}

//  Образы жёсткого диска из одной цепочки должны лежать в одном каталоге.
QString VmDataCollector::getRootFullName(const QString& imageFullName){
    QString rootFullName = imageFullName;
    QString preImageFullName = imageFullName;
    QString backFullName;

    backFullName = getBackFullNameFast(imageFullName);

    while ( backFullName != "-1" ) {
        preImageFullName = backFullName;
        backFullName = getBackFullNameFast(preImageFullName);

        if (backFullName == ""){
            break;
        }

        rootFullName = preImageFullName;
        preImageFullName = backFullName;
    } ;

    return rootFullName;
}

 QString VmDataCollector::getNodeName(const QString& name, const QString& type){

    // Получаем SHA256 хеш от строки (минимизация коллизий в именах)
    QByteArray hash;
    hash = QCryptographicHash::hash(name.toUtf8(), QCryptographicHash::Sha256);

    // Берём первые 4 байта (можно больше, но 4 достаточно для uint)
    quint32 hashValue;
    hashValue = qFromBigEndian<quint32>
                            (reinterpret_cast<const uchar *>(hash.constData()));

    // Получаем индекс по модулю размера списка
    int index = hashValue % nodeNameList.size();

    return nodeNameList[index];
}

void VmDataCollector::process(){
    VMachine result;
    result = getVmInfo(m_vm);
    emit finished(result);
}

VMachine VmDataCollector::getVmInfo(){
    return getVmInfo(m_vm);
}

void VmDataCollector::vmListStartTimer(){
    // TODO Увеличить задержку для прода
    m_getListTimer->start(1000);
}

void VmDataCollector::vmListStopTimer(){
    m_getListTimer->stop();
}

void VmDataCollector::vmListSender(){
    emit vmListReady(getVmList());
}

// End vmDataCollector.cpp
