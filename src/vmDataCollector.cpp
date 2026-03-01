// Begin vmDataCollector.cpp

#include "vmDataCollector.h"

VmDataCollector::VmDataCollector(QWidget* parent) : parentWindow(parent){
    // *** Таймер обновления списка виртуальных машин *** //
    m_getListTimer = new QTimer(this);
    m_getListTimer->setInterval(1000);
    QObject::connect(m_getListTimer, &QTimer::timeout,
                                          this, &VmDataCollector::vmListSender);

    // *** Таймер забора общей информации о ВМ *** //
    m_getActualVmGeneralInfoTimer = new QTimer(this);
    m_getActualVmGeneralInfoTimer->setInterval(1000);
    QObject::connect(m_getActualVmGeneralInfoTimer, &QTimer::timeout,
                            this, &VmDataCollector::selectedVmActualInfoSender);

    // *** Провайдер информации об операционной системе *** //
    m_osInfoProvider = new OsInfoProvider(this);
}

VmDataCollector::VmDataCollector(const VMachine& vm, QWidget* parent) {
    parentWindow = parent;
    m_vm = vm;
    m_osInfoProvider = new OsInfoProvider(this);
}

VmDataCollector::~VmDataCollector(){
}

// Вектор из {name, uuid, state}
QVector<VMachine> VmDataCollector::getVmList(bool* isOk){
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
                process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                          "list", "--all", "--uuid", "--name"});
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

                // В общем случае, между двумя запросами информации число VM
                // может измениться из-за добавления/удаления машин через
                // внешние инструменты virsh/virt-manager и т.д.
                //
                // Предлагается маркировать такой случай для прниятия решения
                // об учёте или не учёте величины
                if (isOk != nullptr){
                    if (vmList.size() != uuidVmList.size()){
                        *isOk = false;
                        vmList.clear();
                    }
                    else {
                        *isOk = true;
                        // *** Слияние информации об uuid VM *** //
                        for (int i = 0; i < vmList.size(); ++i){
                            // *** Очерёдность VM не гарантирована *** //
                            for (int j = 0; j < vmList.size(); ++j){
                                if (vmList[i].name == uuidVmList[j].name){
                                    vmList[i].uuid = uuidVmList[j].uuid;
                                }
                            }
                        }
                    }
                }
                else {
                    vmList.clear();
                }
                loop.quit();
            }
        });

    process.start("virsh", {"--connect=" + m_libVirtConnectURI, "list", "--all"});
    loop.exec();
    return vmList;
}

VMachine VmDataCollector::getVmShortInfo(const QString& uuid, bool* isOk){
    if (isOk != nullptr) {
        *isOk = true;
    }
    VMachine vm;

    // Virtual machine information from XML
    QDomDocument vmXmlDoc = getVmXml(uuid);
    // *** Если ответ пустой, значит VM  с запрошенным uuid не существует *** //
    if (vmXmlDoc.isNull()){
        if (isOk != nullptr) {
            *isOk = false;
        }
        vm.uuid = "";
        vm.name = "";
        return vm;
    }

    QDomElement vmXml = vmXmlDoc.documentElement();
    vm.uuid = uuid;
    vm.name = vmXml.firstChildElement("name").text();
    vm.title = vmXml.firstChildElement("title").text();
    vm.description = vmXml.firstChildElement("description").text();
    vm.ram = vmXml.firstChildElement("memory").text()
                    + " " + vmXml.firstChildElement("memory").attribute("unit");
    QString incomeOsId = vmXml.firstChildElement("metadata")
                                    .firstChildElement("libosinfo:libosinfo")
                                        .firstChildElement("libosinfo:os")
                                            .attribute("id");

    // *** Необходимо для обработки изначально пустого поля *** //
    if (incomeOsId == ""){
        incomeOsId = "Null";
    }

    // *** Из xml описания VM приходит новый id операционной системы *** //
    if (incomeOsId != m_vm.os.id){
        // *** Во входящих данных нет id *** //
        if (incomeOsId == "Null"){
            vm.os.name   = "Not specified";
            vm.os.id     = "";
            vm.os.ram    = "";
            vm.os.vendor = "";
        }
        else {
            vm.os = m_osInfoProvider->getOsInfoByOsId(incomeOsId);
            // *** id во входящих данных есть, но этот id *** //
            //     не изестен текущей libosinfo-db.           //
            //     Вывод пользователю самого id.              //
            if (vm.os.name.isEmpty()){
                vm.os.name = "N/A: " + vm.os.id;
            }
        }
        if (vm.os.id.isEmpty()){
        }
        // *** Чтобы каждый раз при получении нового osId не производить *** //
        //     поиск по всей базе libOsInfo в поисках имени VM, можно        //
        //     сохранить его id и  обновлять имя только, когда будут         //
        //     изменения в osId (первый запуск или изменения свойств VM.     //
        m_vm.os = vm.os;
    }
    else {
        // *** Обновление не требуется, возвращем прежние данные *** //
        vm.os = m_vm.os;
    }

    vm.cpu = vmXml.firstChildElement("vcpu").text() + " (";

    QDomNamedNodeMap cpuAttribues = vmXml.firstChildElement("cpu")
                                    .firstChildElement("topology").attributes();
    for (int i = 0; i < cpuAttribues.count(); ++i ){
        QDomNode attr = cpuAttribues.item(i);
        QString name = attr.nodeName();
        QString value = attr.nodeValue();
        vm.cpu = vm.cpu + name + " " + value + "; ";
    }
    vm.cpu.chop(2);
    if (cpuAttribues.count() > 0) {
        static const QRegularExpression socketsRe(R"(sockets (\d+))");
        static const QRegularExpression coresRe(R"(cores (\d+))");
        static const QRegularExpression threadsRe(R"(threads (\d+))");
        QRegularExpressionMatch sockets = socketsRe.match(vm.cpu);
        QRegularExpressionMatch cores   = coresRe.match(vm.cpu);
        QRegularExpressionMatch threads = threadsRe.match(vm.cpu);
        vm.cpu = sockets.captured() + " · " + cores.captured()
                                                   + " · " + threads.captured();
    }
    else {
        vm.cpu = "sockets " + vm.cpu + " · cores 1 · threads 1";
    }
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
                bool mountStorageErrorFlag = false;
                if (!file.exists()){
                    emit errorMsg("File access error",
                        "Please check your access to the storage file:\n"
                        + QString(" - File name: ") + QFileInfo(mountStorage)
                                                                .fileName()+"\n"
                        + QString(" - Base path: ") + QFileInfo(mountStorage)
                                                               .absolutePath());
                    if (isOk != nullptr) {
                        *isOk = false;
                    }
                    mountStorageErrorFlag = true;
                }

                if (file.isSymLink()){
                    mountStorage = QFileInfo(mountStorage).absolutePath() +"/"+
                                 QFileInfo(file.canonicalFilePath()).fileName();
                }

                if (mountStorageErrorFlag == true){
                    vm.mountStorages.push_back("[x] " + mountStorage);
                }
                else {
                    vm.mountStorages.push_back(mountStorage);
                }

                // *** Загрузка данных о "bus type" для mountStorage *** //
                QString busType = diskNode.firstChildElement("target")
                                                              .attribute("bus");
                vm.driveBusTypes.push_back(busType);

                // *** Загрузка данных о "dev name" для mount Stroage *** //
                QString devName = diskNode.firstChildElement("target")
                                                              .attribute("dev");
                vm.driveDevNames.push_back(devName);

                // *** Установка каталога цепочки сохранения состояния *** //
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

    return vm;
}

VMachine VmDataCollector::getVmFullInfo(const VMachine& vmIn){

    bool isOkLoadShortInfo = true;
    VMachine vm = getVmShortInfo(vmIn.uuid, &isOkLoadShortInfo);
    vm.state = vmIn.state;

    // *** Отмена работы со снапшотами при ошибке загрузки данных *** //
    if (!isOkLoadShortInfo){
        return vm;
    }

    // *** Загрузка данных о qcow2 файлах, из каталогов сохранения *** //
    //     цепочек состояний, с помощью утилиты qemu-img               //
    // *************************************************************** //
    m_vmImagesRawInfo.clear();
    QVector<VmImageRawInfo> vmImagesRawInfo;
    bool isOkLoadRawInfo = true;
    for ( int i = 0; i < vm.snapshotsDirs.size(); ++i ){
        QString dir = vm.snapshotsDirs[i];
        vmImagesRawInfo += loadVmImagesRawInfoOverQEMU(dir, vm.mountStorages,
                                                              &isOkLoadRawInfo);
        // *** Прекращение работы при первой же ошибке получения данных *** //
        if (!isOkLoadRawInfo){
            return vm;
        }
    }

    // *** Проверка на наличие вновь примонтированных хранилищ *** //
    // > Теория. Вновь примонтированными считаются хранилища,      //
    //   у которых в соседях есть хранилища с id, а у самих id     //
    //   отсутсвует. Например, к виртуальной машине добавили диск  //
    //   через VirtManager или иным образом.                       //
    //                                                             //
    // > Проблема добавленные извне файлы проблематично встрочить  //
    //   в уже существующую систему снапшотов. Самый беспроблемый  //
    //   подход состоит в том, чтобы добавляемый диск "размножить" //
    //   на все существующие узлы дерева сохранений, но это надо   //
    //   сделать даже при просмотре информации, что очень криво.   //
    //                                                             //
    // > Решение. На данном этапе надо реализовать запрет          //
    //   на внешнее добавление дисков в какой-либо снапшот,        //
    //   технически можно добавлять хранилища только в корень      //
    //   цепочки сохранения (т.е., необходимо удалить все снапшоты //
    //   и тогда с помощью внешних утилит добавить хранилище),     //
    //   а затем уже строить новое дерево сохранений.              //
    //                                                             //
    // > Далее. Реализовать добавление/удаление дисков виртуальной //
    //   машины из программы с синхронизацией числа дисков по всем //
    //   точкам сохранения (добавление/удаление backing фалов)     //
    // *********************************************************** //

    QStringList extMountedStorages;
    extMountedStorages = getExtMountStorages(vmImagesRawInfo, vm.mountStorages);
    if (extMountedStorages.size() > 0){
        QString extMountStorageStr;
        for (int i = 0; i < extMountedStorages.size(); ++i){
            QString problemImage = extMountedStorages[i];
            extMountStorageStr +=
                        QString("\n - File name: ") + QFileInfo(problemImage)
                                                                .fileName()+"\n"
                      + QString(" - Base path: ") + QFileInfo(problemImage)
                                                               .absolutePath();
        }

        emit errorMsg("Mounted file error",
               "Error. An external attachment the storages has been detected.\n"
                "External attachment within snapshot tree are not allowed."
                   + extMountStorageStr + "\nSnapshot tree build canceled!\n"
               "Try to remove these storage devices from the virtual machine.");
        return vm;
    }

    // ****************************** Теория ******************************  //
    // > Тезис. В цепочке снапшотов необходимо не учитывать ранее созданные  //
    //   цепочки из backing-файлов используемых хранилищ.                    //
    //                                                                       //
    // > Объяснение. Ранее созданные backing-файлы для разных хранилищ       //
    //   не обязаны быть синхронизированы между собой т.е., у одного диска   //
    //   цепочка сохранений может быть  из одного backing-файла, а у другого //
    //   диска из трёх файлов, созданных внешними средствами или вручную.    //
    //                                                                       //
    // > Реализация. Отсечь выше стоящую ветку сохранений по отсутствию id.  //
    //   Добавлять в цепочку сохранений только файлы у которых:              //
    //   - у самих есть id;                                                  //
    //   - у самих нет id, но они примонтированы                             //
    //   - у самих нет id, но являются родителем/backing-файлом              //
    //                   для кого-то с id; (крайний без id, корень цепочки)  //
    //                                                                       //
    // > Этап реализации. После сбора сырых данных в vmImagesRawInfo, этот   //
    //   вектор можно редуцировать с учётом выполнения описанных требований. //
    //   В итоге libvirt-snap-manager будет оперировать крайними точками     //
    //   внешних цепочек сохранения.                                         //
    // ********************************************************************  //

    m_vmImagesRawInfo = rmExtBackingInfo(vmImagesRawInfo, vm.mountStorages);

    // *** Проверка наличия примонтированных файлов в списке файлов *** //
    // > Актуально для случая единственного примонтированного файла     //
    //   без цепочки сохранения и отсутсвии прав доступа на чтение      //
    //   у данной программы. Виртуальная машина будет работать,         //
    //   а дерево снапшотов не построится т.к., к файлу нет доступа.    //
    //                                                                  //
    // > Проверка должна проводится по черновым данным т.к., после      //
    //   rmExtBackingInfo(), если где-то была "потеря", то отбросится   //
    //   вся цепочка вплоть до актуальной точки монтирования на дальней //
    //   ветке дерева сохранения состояний.                             //
    // **************************************************************** //

    for (int i = 0; i < vm.mountStorages.size(); ++i){
        int isLostMountFile = true;
        for (int j = 0; j < vmImagesRawInfo.size(); ++j){
            if (vm.mountStorages[i] == vmImagesRawInfo[j].imageFullName){
                isLostMountFile = false;
                break;
            }
        }

        if (isLostMountFile){
            emit errorMsg("File access error",
                "Please check your access to the VM image file:\n"
                    + QString(" - File name: ")
                        + QFileInfo(vm.mountStorages[i]).fileName()+"\n"
                    + QString(" - Base path: ")
                        + QFileInfo(vm.mountStorages[i]).absolutePath());
            return vm;
        }
    }

    // *** Пропуск действий, если ранее был получен пустой ответ *** //
    //     Пустым ответ приходит, когда в rmExtBackingInfo() есть    //
    //     потери файлов, ожидаемых в цепочке сохранения состяний.   //
    // ************************************************************* //
    if (m_vmImagesRawInfo.size() == 0){
        return vm;
    }

    // Определение корневых файлов для каждой цепочки сохранения
    for (int i = 0; i < vm.mountStorages.size(); ++i){
        vm.rootFullName.push_back(getRootFullName(vm.mountStorages[i]));
    }

    // Определение виртуального (внутреннего) размера хранилищ данных
    for (int i = 0; i < vm.mountStorages.size(); ++i){
        vm.rootVirtSizes.push_back(getRootVirtSize(vm.rootFullName[i]));
    }

    // Построение цепочек сохранённых состояний
    setSnapChainData(vm);

    // Заполнение данных о потомках каждого узла
    // (необходимо для реализации удаления узла внутри цепочки)
    setChildrenData(vm);

    return vm;
}

QDomDocument VmDataCollector::getVmXml(const QString& uuid){
    QDomDocument vmXmlDoc;

    QEventLoop loop;
    QProcess process;

    QObject::connect(&process, &QProcess::finished, [&](){

            QString output = process.readAllStandardOutput();
            if (!vmXmlDoc.setContent(output)) {
                // *** Ошибка парсинга, возврат пустого документа *** //
                vmXmlDoc.clear();
            }
            loop.quit();
        });

    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                              "dumpxml", uuid});
    loop.exec();

    return vmXmlDoc;
}

void VmDataCollector::setSnapChainData(VMachine& vm){

    // Данные для корневой точки;
    int id = 1;
    int parentId = -1;
    while (true){
        ChainNode node;
        node.imagesType = "work";
        bool usedVmUUID = false;
        int rootStoragesCounter = 0;
        for (int mntIdx = 0; mntIdx  < vm.mountStorages.size(); ++mntIdx){
            // Формирование корневого узла в цепочке сохранений;
            if (parentId == -1){
                rootStoragesCounter++;
                node.id = id;
                node.parentId = parentId;
                node.imagesType = "active";
                // Если у диска есть потомки, то он не может быть изменён
                // т.е., выполняет роль хранителя состояния, снимка,
                // его тип "snap". По умолчанию тип состояния "work"
                QString imageFullName = vm.rootFullName[mntIdx];
                // Есть ли у корневого диска потомки?
                for (int i = 0; i < m_vmImagesRawInfo.size(); ++i){
                    if (imageFullName == m_vmImagesRawInfo[i].backFullName){
                        node.imagesType = "snap";
                        break;
                    }
                }
                node.imagesFullNames.push_back(imageFullName);
                node.backFullNames.push_back("None");

                // *** Проверка того, что в корне есть диск без id *** //
                // Если в одном из случаев не найден файл id, то надо  //
                // испльзовать vm.uuid, в противном случае использвать //
                // надо id файла. Таким образом, при удалении снапшота //
                // и перемещении N+1 на место корня, его UUID останется//
                // валидным.                                           //
                // *************************************************** //
                static const QRegularExpression re(R"(-id-(\d{10}))");
                QRegularExpressionMatch match = re.match(imageFullName);

                if (!match.hasMatch()){
                    usedVmUUID = true;
                }

                if (rootStoragesCounter == vm.mountStorages.size()){
                    node.uuid = getNodeUuid(imageFullName, usedVmUUID);
                    node.name = getNodeName(node.uuid);
                    rootStoragesCounter = 0;
                }
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

                        node.uuid = getNodeUuid(imageFullName);
                        node.name = getNodeName(node.uuid);

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
        // *** К одному из файлов в каталоге снапшотов нет доступа *** //
        return false;
    }

    QByteArray header = file.read(4);

    if (header == QByteArray::fromHex("514649FB")){
       vmImage = true;
    }

    return vmImage;
}

QVector<VmImageRawInfo> VmDataCollector::loadVmImagesRawInfoOverQEMU(
                                                    const QString& snapshotsDir,
                                                    const QStringList& mntImgs,
                                                         bool* isOkLoadRawInfo){
    QVector<VmImageRawInfo> vmImagesRawInfo;

    // Получение списка всех файлов из каталога цепочки сохранения состояний
    // /* только имена файлов */
    QStringList basePathFiles = QDir(snapshotsDir).entryList(QDir::Files
                                | QDir::NoSymLinks | QDir::Hidden, QDir::Name);

    // Проверка на доступность (файлы всегда должны быть по логике программы)
    if (basePathFiles.size() == 0){
        qDebug() << "[EE] Error open directory:" << snapshotsDir;
        emit errorMsg("Directory access error",
                 "Please check your access to the directory:\n" + snapshotsDir);
        *isOkLoadRawInfo = false;
        return vmImagesRawInfo;
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
        // *** mntImgs.contains(fileFullName) отрабатывет случай, когда *** //
        //     в виртуальной машине файлы недоступны для чтения текущему    //
        //     пользователю, например, при первом запуске программы         //
        if (isVMachineImage(fileFullName) || mntImgs.contains(fileFullName)){
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
            vmImagesRawInfo.push_back(vmImageRawInfo);
        }
    }
    return vmImagesRawInfo;
}

QVector<VmImageRawInfo> VmDataCollector::rmExtBackingInfo(
                                const QVector<VmImageRawInfo>& imgsRawInfo,
                                            const QStringList& mountStorages) {
    // > Реализация. Отсечь выше стоящую ветку сохранений по отсутствию id.  //
    //   Добавлять в цепочку сохранений только файлы у которых:              //
    //   - у самих есть id;                                                  //
    //   - у самих нет id, но они примонтированы                             //
    //   - у самих нет id, но являются родителем/backing-файлом              //
    //                   для кого-то с id; (крайний без id, корень цепочки)  //

    QVector<VmImageRawInfo> res;
    QSet<int> idSet;

    for (int i = 0; i < imgsRawInfo.size(); ++i){
        QString imageFullName = imgsRawInfo[i].imageFullName;
        idSet.insert(getFileNameId(imageFullName));
        // есть id
        if (getFileNameId(imageFullName) > 0){
            // *** Проверка на существования родителя добавляемого файла *** //
            bool ok = false;
            ok = checkBackingFile(imgsRawInfo, imgsRawInfo[i].backFullName);

            if (ok){
                // *** Родитель существует, всё хорошо *** //
                res.push_back(imgsRawInfo[i]);
            }
            else {
                // *** Родитель не найден, возрват пустых данных *** //
                res.clear();
                return res;
            }
        }
        // нет id
        else {
            bool mountFlag = false;
            for (int j = 0; j < mountStorages.size(); ++j){
                // нет id, но примонтированы к VM
                if (imageFullName == mountStorages[j]){
                    // *** Проверка внешней скрываемой цепочки *** //
                    QString backFullName  = imgsRawInfo[i].backFullName;
                    bool ok = true;
                    ok = checkExtBackChainFiles(imgsRawInfo, backFullName);

                    if (ok){
                        // *** Скрываемая цепочка файлов в порядке *** //
                        res.push_back(imgsRawInfo[i]);
                        mountFlag = true;
                    }
                    else {
                        // Ошибки в скрываемой цепочке, возрат пустых данных
                        res.clear();
                        return res;
                    }
                }
            }

            if (!mountFlag){
                for (int k = 0; k < imgsRawInfo.size(); ++k){
                    QString childBackingFile = imgsRawInfo[k].backFullName;
                    QString childImageFullName = imgsRawInfo[k].imageFullName;
                    // нет id, но являются родителем для кого-то с id;
                    if (imageFullName == childBackingFile
                                     && getFileNameId(childImageFullName) > 0 ){
                        QString backFullName  = imgsRawInfo[i].backFullName;
                        bool ok = true;
                        ok = checkExtBackChainFiles(imgsRawInfo, backFullName);

                        if (ok){
                            // *** Скрываемая цепочка файлов в порядке *** //
                            res.push_back(imgsRawInfo[i]);
                            res.last().backFullName = "-1";
                        }
                        else {
                            // Ошибки в скрываемой цепочке, возрат пустых данных
                            res.clear();
                            return res;
                        }
                    }
                }
            }
        }
    }

    // *** Проверка числа файлов в снапшотах (актуально для "листьев") *** //
    bool isOkNodeFiles = checkNodeFilesCount(imgsRawInfo, idSet, mountStorages);
    if (!isOkNodeFiles){
        res.clear();
    }

    return res;
}

QString VmDataCollector::getBackFullNameQEMU(const VmImageRawInfo& vmImgRawInf){
    QString backingFile = "-1";

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

QString VmDataCollector::getRootVirtSize(const QString& imageFullName){
    QString res;

    QProcess process;
    QEventLoop loop;

    QObject::connect(&process, &QProcess::finished,
        [&](){
            QString output = process.readAllStandardOutput();
            QStringList outputLines = output.split('\n',Qt::SkipEmptyParts);

            // third line is "virtual size: 20 GiB (21474836480 bytes)"
            if (outputLines.size() > 4){
                QStringList parts = outputLines[2].split(" ");
                res = parts[2] + " " + parts[3];
            }
            else {
                output = process.readAllStandardError();
                if (output.contains("Permission denied")){
                    // *** Ситуация, когда к виртуальной машине подключен *** //
                    //     диск с правами -rw-------, прав на чтение данных   //
                    //     через qemu-img info <imageFullName> нет. Далее     //
                    //     попытка получить данные через virsh vol-info ...   //
                    QStringList poolList = this->getPoolList();
                    res = getVirtSize(imageFullName, poolList);
                }
                else {
                    res = "??? GiB";
                }
            }

            loop.quit();
        });

    process.start("qemu-img", {"info", "--force-share", imageFullName});
    loop.exec();

    return res;
}

QStringList VmDataCollector::getPoolList(){
    QStringList res;

    QProcess process;
    QEventLoop loop;

    QObject::connect(&process, &QProcess::finished,
        [&](){
            QString output = process.readAllStandardOutput();
            QStringList outputLines = output.split('\n',Qt::SkipEmptyParts);
            for (int i = 0; i < outputLines.size(); ++i){
                res.push_back(outputLines[i]);
            }
            loop.quit();
            });

    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                               "pool-list", "--all", "--name"});
    loop.exec();

    return res;
}

QString VmDataCollector::getVirtSize(const QString& imageFullName,
                                                   const QStringList& poolList){
    QString res = "--- GiB";

    for (int i = 0; i < poolList.size(); ++i){
        QProcessEnvironment env;
        env.insert("LANG", "C");

        QProcess process;
        process.setProcessEnvironment(env);
        QEventLoop loop;

        QObject::connect(&process, &QProcess::finished,
            [&](){
                QString output = process.readAllStandardOutput();
                QStringList outputLines = output.split('\n',Qt::SkipEmptyParts);
                for (int i = 0; i < outputLines.size(); ++i){
                    if (outputLines[i].contains("Capacity")){
                        // *** Округление размера до целого числа *** //
                        QString noRoundRes = outputLines[i]
                               .split(':', Qt::SkipEmptyParts).last().trimmed();
                        QStringList resList = noRoundRes.split(' ');
                        int valueRes = qRound(resList.first().toFloat());
                        res = QString::number(valueRes) + " " + resList.last();
                    }
                }
                loop.quit();
                });

        process.start("virsh", {"--connect=" + m_libVirtConnectURI, "vol-info",
                                         "--pool", poolList[i], imageFullName});
        loop.exec();
    }

    return res;
}

QString VmDataCollector::getNodeName(const QString& imageFullName){

    // Получаем SHA256 хеш от строки (минимизация коллизий в именах)
    QByteArray hash;
    hash = QCryptographicHash::hash(imageFullName.toUtf8(),
                                                    QCryptographicHash::Sha256);

    // Берём первые 4 байта (можно больше, но 4 достаточно для uint)
    quint32 hashValue;
    hashValue = qFromBigEndian<quint32>
                            (reinterpret_cast<const uchar *>(hash.constData()));

    // Получаем индекс по модулю размера списка
    int index = hashValue % nodeNameList.size();

    return nodeNameList[index];
}

QString VmDataCollector::getNodeUuid(const QString& fName, bool isRoot){
    // *** Чтобы UUID узла не зависел от конкретного имени файла *** //
    // UUID будет рассчитываться на базе id из файла "0123456789",   //
    // У корневых файлов id нет, там за основу будет взят uuid       //
    // виртуальной машины.                                           //
    // ************************************************************* //
    QString baseData = "baseDataExapmle";

    if (isRoot){
        baseData = m_vm.uuid;
    }
    else {
        static const QRegularExpression re(R"(-id-(\d{10}))");
        QRegularExpressionMatch match = re.match(fName);

        if (match.hasMatch()){
            baseData = match.captured(1);
        }
        else {
            // *** Корректное определение текущего id при создании *** //
            //           первого снапшота от корневого файла.          //
            // ******************************************************* //
            baseData = m_vm.uuid;
        }
    }

    QUuid uuid = QUuid::createUuidV5(QUuid::fromString(
                                    "{12329e42e-d24e-4ad0-84dd-9e03b8b33e7}"),
                                                                      baseData);
    return uuid.toString(QUuid::WithoutBraces);
}

QString VmDataCollector::getNextBlockDevice(const QString& vmName,
                                                        const QString& busType){
    QString newBlockDevice;

    QEventLoop loop;
    QProcess process;

    QObject::connect(&process, &QProcess::finished,
        [&](int, QProcess::ExitStatus){
            QString output = process.readAllStandardOutput();
            QStringList outputLines = output.split('\n',Qt::SkipEmptyParts);

            QStringList allBlockDev;
            for (int i = 2; i < outputLines.size(); ++i){
                QString line = outputLines[i].trimmed();
                QStringList parts = line.split(" ");
                allBlockDev.push_back(parts.first());
            }

            QString devPrefix;
            if (busType.toLower() == "virtio"){
                devPrefix = "vd";
            }
            else {
                devPrefix = "sd";
            }

            QVector<int> busyLetterIds;
            for (int i = 0; i < allBlockDev.size(); ++i){
                QString blkDev = allBlockDev[i];
                if (blkDev.startsWith(devPrefix)){
                    int blkLetterIndex = blkDev[2].toLower().toLatin1() - 'a';
                    busyLetterIds.push_back(blkLetterIndex);
                }
            }

            int maxBusyLetterIndex = -1;
            if (busyLetterIds.size() < 26){
                for (int i = 0; i < busyLetterIds.size(); ++i){
                    if (busyLetterIds[i] > maxBusyLetterIndex) {
                        maxBusyLetterIndex = busyLetterIds[i];
                    }
                }
            }
            else {
                qDebug() << "[II] No free block devices";
                newBlockDevice = "";
                loop.quit();
            }

            int newLetterIndex = maxBusyLetterIndex + 1;
            QChar newLetter = QChar('a' + newLetterIndex);
            newBlockDevice = devPrefix + newLetter;
            loop.quit();
        });

    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                         "domblklist", vmName});
    loop.exec();
    return newBlockDevice;
}

int VmDataCollector::getFileNameId(const QString& imgFileName){
    int id = -1;

    static const QRegularExpression re(R"(-id-(\d{10}))");
    QRegularExpressionMatch match = re.match(imgFileName);

    if (match.hasMatch()){
        id = match.captured(1).toInt();
    }

    return id;
}

bool VmDataCollector::checkExtBackChainFiles(const QVector<VmImageRawInfo>&
                                       imgsRawInfo, const QString& backingFile){
    bool res = true;
    QString bName = backingFile;

    while (bName != "-1"){
        bool backingFileFlag = false;
        for (int i = 0; i < imgsRawInfo.size(); ++i){
            if (bName == imgsRawInfo[i].imageFullName){
                bName = imgsRawInfo[i].backFullName;
                backingFileFlag = true;
                break;
            }
        }

        if (!backingFileFlag){
            res = false;
            emit errorMsg("File access error",
                     "Please check your access to the backing file:\n"
                        + QString(" - File name: ") + QFileInfo(bName)
                                                                .fileName()+"\n"
                        + QString(" - Base path: ") + QFileInfo(bName)
                                                               .absolutePath());
            break;
        }
    }

    return res;
}

bool VmDataCollector::checkBackingFile(const QVector<VmImageRawInfo>&
                                       imgsRawInfo, const QString& backingFile){
    bool res = true;
    // *** Родителя не существует, возвращение корректной проверки *** //
    if (backingFile == "-1"){
        return true;
    }

    bool backingFileFlag = false;
    for (int i = 0; i < imgsRawInfo.size(); ++i){
        if (backingFile == imgsRawInfo[i].imageFullName){
            backingFileFlag = true;
            break;
        }
    }

    if (!backingFileFlag){
        res = false;
        emit errorMsg("File access error",
               "Please check your access to the backing file:\n"
                        + QString(" - File name: ") + QFileInfo(backingFile)
                                                                .fileName()+"\n"
                        + QString(" - Base path: ") + QFileInfo(backingFile)
                                                               .absolutePath());
    }

    return res;
}

bool VmDataCollector::checkNodeFilesCount(
                                const QVector<VmImageRawInfo>& imgsRawInfo,
                                const QSet<int>& idSet,
                                const QStringList& mountStorages){

    // ****************************** Теория ******************************  //
    // > Тезис. Для корректного построения цепочек сохранения, требуется     //
    //   наличие всех файлов, соответствующих используемым хранилищам.       //
    //                                                                       //
    // > Объяснение. Если какого-то файла нет, то цепочка сохранений для     //
    //   текущего хранилища обрывается, появляется рассинхронизация.         //
    //                                                                       //
    // > Решение. Определять отсутствущий файл и при обнаружении отсутствия  //
    //   не строить цепочку сохранений с выводом информации о проблеме.      //
    //                                                                       //
    // > Реализация. У всех файлов снапшотов, кроме корневых есть id. Для    //
    //   таких файлов можно создать множество уникальных id и проверять      //
    //   число файлов с таким id, оно должно быть равно количеству хранилищ. //
    //   Для корневых файлов можно определить число файлов без id, оно       //
    //   после предыдущей очистки, тоже должно быть равно числу хранилищ ВМ. //
    // ********************************************************************  //

    bool res = true;

    for (QSet<int>::const_iterator it = idSet.begin(); it != idSet.end(); ++it){
        int id  = *it;
        int problemId;
        // *** "-1" всегда выявится на проверке backing файла, пропуск *** //
        if (id == -1){
            continue;
        }

        int counter = 0;
        bool skipRoot = false;
        for (int i = 0; i < imgsRawInfo.size(); ++i){
            // *** Даже если у файла есть физически id, но по факту *** //
            //     у него нет родителей, то он корневой и его id = -1   //
            int tmpId;
            if (imgsRawInfo[i].backFullName == "-1"){
                tmpId = -1;
                skipRoot = true;
            }
            else {
                tmpId = getFileNameId(imgsRawInfo[i].imageFullName);
            }

            if (tmpId == id && tmpId != -1){
                counter++;
            }
        }

        if (skipRoot){
            continue;
        }

        if (counter != mountStorages.size()){
            problemId = id;
            QStringList existImageFullName;

            // Формирование списка существующих файлов с проблемным id
            for (int j = 0; j < imgsRawInfo.size(); ++j){
                if (problemId == getFileNameId(imgsRawInfo[j].imageFullName)){
                    existImageFullName.push_back(imgsRawInfo[j].imageFullName);
                }
            }

            for (int k = 0; k < mountStorages.size(); ++k){
                QString mountStorage = mountStorages[k];
                // *** Выдление общей части в названии файла *** //
                mountStorage.remove(QRegularExpression("-id-\\d{10}"));
                if (mountStorage.endsWith(".qcow2", Qt::CaseInsensitive)){
                    mountStorage.chop(QString(".qcow2").size());
                }

                bool inside = false;
                for (int m = 0; m < existImageFullName.size(); ++m){
                    if (existImageFullName[m].contains(mountStorage)) {
                        inside = true;
                    }
                }

                // Если общая часть названия точки монтирования не входит
                // в существующие файлы с проблемным id, значит файл потерян
                if (!inside){
                    QString problemImage = mountStorage + "-id-"
                                        + QString::number(problemId) + ".qcow2";
                    emit errorMsg("File access error",
                     "Please check your access to the workpoint file:\n"
                        + QString(" - File name: ") + QFileInfo(problemImage)
                                                                .fileName()+"\n"
                        + QString(" - Base path: ") + QFileInfo(problemImage)
                                                               .absolutePath());
                }
            }

            res = false;
            break;
        }
    }
    return res;
}

void VmDataCollector::process(){
    // До getVmFullInfo() в m_vm хранится "скелет" ВМ (name, state, uuid)
    // result - полная версия ВМ с цепочками сохранения и используемыми дискми
    VMachine result;
    result = getVmFullInfo(m_vm);
    emit finished(result);
}

VMachine VmDataCollector::getVmFullInfo(){
    return getVmFullInfo(m_vm);
}

void VmDataCollector::vmListStartTimer(){
    m_getListTimer->start();
}

void VmDataCollector::vmListStopTimer(){
    m_getListTimer->stop();
}

void VmDataCollector::vmListSender(){
    bool isOkVmList = true;
    QVector<VMachine> vmList = getVmList(&isOkVmList);
    // Если в двух запросах uuid & name число VM разное,
    // то результат отбрасывается т.к., данные не консистентны
    if (isOkVmList){
        emit vmListReady(vmList);
    }
}

void VmDataCollector::vmGeneralInfoStartTimer(const VMachine& vm){
    m_vm.uuid = vm.uuid;
    m_getActualVmGeneralInfoTimer->start();
}

void VmDataCollector::vmGeneralInfoStartTimer(){
    m_getActualVmGeneralInfoTimer->start();
}

void VmDataCollector::vmGeneralInfoStopTimer(){
    m_getActualVmGeneralInfoTimer->stop();
}

void VmDataCollector::selectedVmActualInfoSender(){
    bool isOkLoadShortInfo = false;
    VMachine vm = getVmShortInfo(m_vm.uuid, &isOkLoadShortInfo);
    emit newVmInfoReady(vm);
}

void VmDataCollector::getSnapshotXmlInfo(ChainNode& node){
    // Virtual machine information in XML
    QDomDocument vmXmlDoc = getVmXml(m_vm.uuid);
    QDomElement vmXml = vmXmlDoc.documentElement();

    QDomElement snapInfo = vmXml.firstChildElement("metadata")
                        .firstChildElement("libvirt-snapshot-manager:snapInfo");
    QDomNodeList snapshots = snapInfo.elementsByTagName("snapshot");

    // *** Поиск элемента с атрибутом uuid равным node.uuid *** //
    QDomElement snapshotByUuid;
    for (int i = 0; i < snapshots.size(); ++i) {
        QDomElement xmlElement = snapshots.item(i).toElement();
        if (!xmlElement.isNull() && xmlElement.hasAttribute("uuid")) {
            if (xmlElement.attribute("uuid") == node.uuid) {
                snapshotByUuid = xmlElement;
                break;
            }
        }
    }

    node.title = snapshotByUuid.firstChildElement("title").text();
    node.description = snapshotByUuid.firstChildElement("description").text();
}

void VmDataCollector::writeVmTitle(const QString& vm_uuid,
                                                     const QString& newVmTitle){
    QDomDocument vmXmlDoc = getVmXml(vm_uuid);
    QDomElement  vmXml = vmXmlDoc.documentElement();
    QDomElement titleVmXml = vmXml.firstChildElement("title");

    // *** Случай,  когда поле Title не существовало *** //
    if (titleVmXml.isNull()) {
        QDomElement newTitleVmXml = vmXmlDoc.createElement("title");
        QDomText    newTitleVmXmlText = vmXmlDoc.createTextNode(newVmTitle);
        newTitleVmXml.appendChild(newTitleVmXmlText);
        vmXml.appendChild(newTitleVmXml);
    }
    else{
        // Справочно. titleVmXml — это элемент <title>. У элемента
        // есть дочерний текстовый узел (QDomText), в котором
        // реально хранится строка. setNodeValue() для элемента
        // (QDomElement) ничего не меняет, потому что значение
        // текста хранится в его child-узле, а не в самом элементе.
        titleVmXml.firstChild().setNodeValue(newVmTitle);
    }

    pushVmXml(vmXmlDoc);

}

void VmDataCollector::writeVmDescription(const QString& vm_uuid,
                                               const QString& newVmDescription){
    QDomDocument vmXmlDoc = getVmXml(vm_uuid);
    QDomElement  vmXml = vmXmlDoc.documentElement();
    QDomElement descriptionVmXml = vmXml.firstChildElement("description");

    // *** Случай,  когда поле Title не существовало *** //
    if (descriptionVmXml.isNull()) {
        QDomElement newDescriptionVmXml = vmXmlDoc.createElement("description");
        QDomText    newDescriptionVmXmlText = vmXmlDoc
                                              .createTextNode(newVmDescription);
        newDescriptionVmXml.appendChild(newDescriptionVmXmlText);
        vmXml.appendChild(newDescriptionVmXml);
    }
    else{
        // Справочно. titleVmXml — это элемент <title>. У элемента
        // есть дочерний текстовый узел (QDomText), в котором
        // реально хранится строка. setNodeValue() для элемента
        // (QDomElement) ничего не меняет, потому что значение
        // текста хранится в его child-узле, а не в самом элементе.
        descriptionVmXml.firstChild().setNodeValue(newVmDescription);
    }

    pushVmXml(vmXmlDoc);
}

void VmDataCollector::writeSnapTitle(const QStringList& uuid,
                                                   const QStringList& snapInfo){
    QString vm_uuid = uuid[0];
    QString snap_uuid = uuid[1];
    QString snapName = snapInfo[0];
    QString snapTitle = snapInfo[1];

    QDomDocument vmXmlDoc = getVmXml(vm_uuid);

    QDomElement  vmXml = vmXmlDoc.documentElement();

    QDomElement metadataXml = findOrCreateElement(vmXmlDoc, vmXml, "metadata");

    QDomElement snapInfoXml = findOrCreateElement(vmXmlDoc, metadataXml,
                                           "libvirt-snapshot-manager:snapInfo");

    snapInfoXml.setAttribute("xmlns:libvirt-snapshot-manager",
                      "https://github.com/Jurik-Phys/libvirt-snapshot-manager");

    QDomElement snapshotXml = findOrCreateSnapshotElement(vmXmlDoc, snapInfoXml,
                                                                     snap_uuid);
    QDomElement snapNameXml = findOrCreateElement(vmXmlDoc, snapshotXml,
                                                                        "name");
    QDomElement snapTitleXml = findOrCreateElement(vmXmlDoc, snapshotXml,
                                                                       "title");

    writeQDomElementText(vmXmlDoc, snapNameXml, snapName );
    writeQDomElementText(vmXmlDoc, snapTitleXml, snapTitle );

    pushVmXml(vmXmlDoc);
}

void VmDataCollector::writeSnapDescription(const QStringList& uuid,
                                                   const QStringList& snapInfo){
    QString vm_uuid = uuid[0];
    QString snap_uuid = uuid[1];
    QString snapName = snapInfo[0];
    QString snapDescription = snapInfo[1];

    QDomDocument vmXmlDoc = getVmXml(vm_uuid);

    QDomElement  vmXml = vmXmlDoc.documentElement();

    QDomElement metadataXml = findOrCreateElement(vmXmlDoc, vmXml, "metadata");

    QDomElement snapInfoXml = findOrCreateElement(vmXmlDoc, metadataXml,
                                           "libvirt-snapshot-manager:snapInfo");

    snapInfoXml.setAttribute("xmlns:libvirt-snapshot-manager",
                      "https://github.com/Jurik-Phys/libvirt-snapshot-manager");

    QDomElement snapshotXml = findOrCreateSnapshotElement(vmXmlDoc, snapInfoXml,
                                                                     snap_uuid);
    QDomElement snapNameXml = findOrCreateElement(vmXmlDoc, snapshotXml,
                                                                        "name");
    QDomElement snapDescriptionXml = findOrCreateElement(vmXmlDoc, snapshotXml,
                                                                 "description");

    writeQDomElementText(vmXmlDoc, snapNameXml, snapName );
    writeQDomElementText(vmXmlDoc, snapDescriptionXml, snapDescription );

    pushVmXml(vmXmlDoc);
}

void VmDataCollector::rmSnapshotXmlElement(const QStringList& uuid){

    QString vmUuid = uuid[0];
    QString snapUuid = uuid[1];

    QDomDocument vmXmlDoc = getVmXml(vmUuid);

    QDomElement  vmXml = vmXmlDoc.documentElement();

    QDomElement metadataXml = vmXml.firstChildElement("metadata");

    // *** Нет элемента metadata, значит нет и всех остальных элементов *** //
    if (metadataXml.isNull()){
        return;
    }

    QDomElement snapInfoXml = metadataXml
                        .firstChildElement("libvirt-snapshot-manager:snapInfo");

    // *** Нет раздела со снапшотами, удалять нечего *** //
    if (snapInfoXml.isNull()){
        return;
    }

    QDomElement toRemoveSnapshot;
    QDomNodeList snapshots = snapInfoXml.elementsByTagName("snapshot");
    for (int i = 0; i < snapshots.size(); ++i){
        QDomElement tmpSnap = snapshots.at(i).toElement();
        if (!tmpSnap.isNull() && tmpSnap.attribute("uuid") == snapUuid){
            toRemoveSnapshot = tmpSnap;
        }
    }

    if (toRemoveSnapshot.isNull()){
        return;
    }
    else {
        // *** Удаление единственного снапошота *** //
        if (snapshots.size() == 1){
            metadataXml.removeChild(snapInfoXml);
        }
        else {
            snapInfoXml.removeChild(toRemoveSnapshot);
        }
    }

    pushVmXml(vmXmlDoc);
}

void VmDataCollector::pushVmXml(const QDomDocument& vmXmlDoc){
    QEventLoop loop;
    QProcess process;

    QDomElement vmXml = vmXmlDoc.documentElement();
    QString vm_uuid = vmXml.firstChildElement("uuid").text();

    // *** Сохранение временного xml файла *** //
    QString fileName = "/tmp/" + vm_uuid + ".xml";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text
                | QIODevice::Truncate)){
        qDebug() << "[EE] Failed to write back XML file.";
        return;
    }

    // *** "4" пробела для отступа при сериализации XML *** //
    QTextStream out(&file);
    vmXmlDoc.save(out, 4);
    file.close();

    QObject::connect(&process, &QProcess::finished,
        [&](){
           // *** Удаление временного xml файла *** //
           if (QFile::exists(fileName)) {
               if (!QFile::remove(fileName)) {
                   qDebug() << "[EE] don't delete:" << fileName;
               }
           }
           loop.quit();
        });

    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                           "define", fileName});
    loop.exec();
}

QDomElement VmDataCollector::findOrCreateElement(QDomDocument& doc,
                                       QDomElement& parent, const QString& tag){
    QDomElement res = parent.firstChildElement(tag);
    if (res.isNull()){
        res = doc.createElement(tag);
        parent.appendChild(res);
    }
    return res;
}

QDomElement VmDataCollector::findOrCreateSnapshotElement(QDomDocument& doc,
                                QDomElement& snapInf, const QString& snap_uuid){
    QDomElement res;

    QDomNodeList snapshots = snapInf.elementsByTagName("snapshot");
    for (int i = 0; i < snapshots.size(); ++i){
        QDomElement tmpSnap = snapshots.at(i).toElement();
        if (!tmpSnap.isNull() && tmpSnap.attribute("uuid") == snap_uuid){
            res = tmpSnap;
        }
    }

    if (res.isNull()){
        res = doc.createElement("snapshot");
        res.setAttribute("uuid", snap_uuid);
        snapInf.appendChild(res);
    }

    return res;
}

void VmDataCollector::writeQDomElementText(QDomDocument& doc, QDomElement& el,
                                                          const QString& value){
    if (!el.isNull() && el.firstChild().isText()){
        el.firstChild().setNodeValue(value);
    }
    else {
        QDomText text = doc.createTextNode(value);
        el.appendChild(text);
    }
}

QStringList VmDataCollector::getExtMountStorages(
                                const QVector<VmImageRawInfo>& imgsRawInfo,
                                            const QStringList& mountStorages) {
    QStringList res;
    int id = -1;

    for (int i = 0; i < mountStorages.size(); ++i){
        id = getFileNameId(mountStorages[i]);
        if (id != -1){
            // *** Файл с id может быть корнем цепочки сохранения *** //
            // Необходимо проверить, наличие у него родителей, если   //
            // родителей нет, то это действительно корень и сброс     //
            // id, полученного из имени файла, в "-1"                 //
            // ****************************************************** //
            for (int k = 0; k < imgsRawInfo.size(); ++k){
                if (imgsRawInfo[k].imageFullName == mountStorages[i]){
                    QString backFullName = imgsRawInfo[k].backFullName;
                    if (backFullName == "-1"){
                        id = -1;
                    }
                }
            }
            break;
        }
    }

    if (id != -1){
        for (int i = 0; i < mountStorages.size(); ++i){
            if (getFileNameId(mountStorages[i]) == -1){
                res.push_back(mountStorages[i]);
            }
        }
    }

    return res;
}

void VmDataCollector::setData(const VMachine& vm){
    m_vm = vm;
}

void VmDataCollector::onRequestVmOsInfoUpdate(){
    m_osInfoProvider->reloadData();
    m_vm.os = m_osInfoProvider->getOsInfoByOsId(m_vm.os.id);
}

void VmDataCollector::onRequestVmOsXmlInfoClear(){
    QDomDocument vmXmlDoc = getVmXml(m_vm.uuid);
    QDomElement  vmXml = vmXmlDoc.documentElement();
    QDomElement metadataXml = vmXml.firstChildElement("metadata");
    if (metadataXml.isNull()){
        return;
    }

    QDomElement libOsInfoXml = metadataXml
                                      .firstChildElement("libosinfo:libosinfo");
    if (libOsInfoXml.isNull()){
        return;
    }
    else {
        metadataXml.removeChild(libOsInfoXml);
    }
    pushVmXml(vmXmlDoc);
}

void VmDataCollector::onRequestVmOsXmlInfoUpdate(const OsInfo& osInfo){
    QString libOsInfoNamespace;
    libOsInfoNamespace = "http://libosinfo.org/xmlns/libvirt/domain/1.0";

    QDomDocument vmXmlDoc = getVmXml(m_vm.uuid);
    QDomElement  vmXml = vmXmlDoc.documentElement();
    QDomElement  metadataXml = findOrCreateElement(vmXmlDoc, vmXml, "metadata");
    QDomElement  libOsInfoXml = findOrCreateElement(vmXmlDoc, metadataXml,
                                                         "libosinfo:libosinfo");
    libOsInfoXml.setAttribute("xmlns:libosinfo", libOsInfoNamespace);
    QDomElement libOsIdXml = findOrCreateElement(vmXmlDoc, libOsInfoXml,
                                                                "libosinfo:os");
    libOsIdXml.setAttribute("id", osInfo.id);
    pushVmXml(vmXmlDoc);
}

void VmDataCollector::onRequestVmRamXmlUpdate(const long int& memoryInKiB){

    QDomDocument vmXmlDoc = getVmXml(m_vm.uuid);
    QDomElement  vmXml = vmXmlDoc.documentElement();
    QDomElement  memoryXml = findOrCreateElement(vmXmlDoc, vmXml, "memory");
    QDomElement  currentMemoryXml = findOrCreateElement(vmXmlDoc, vmXml,
                                                               "currentMemory");
    memoryXml.setAttribute("units", "KiB");
    currentMemoryXml.setAttribute("units", "KiB");

    QString memoryValueString = QString::number(memoryInKiB);
    writeQDomElementText(vmXmlDoc, memoryXml, memoryValueString);
    writeQDomElementText(vmXmlDoc, currentMemoryXml, memoryValueString);

    pushVmXml(vmXmlDoc);
}

// End vmDataCollector.cpp
