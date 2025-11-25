// Begin snapManager.cpp

#include "snapManager.h"

SnapManager::SnapManager(QWidget* parent) : parentWindow(parent){
}

SnapManager::~SnapManager(){
}

QStringList SnapManager::doSnapshot(const QString& vmName,
                                    const QString& snapName,
                                    const QStringList& workDisks, bool silence){
    // *** Подтверждение создания снапшота *** //
    if (!silence) {
        QMessageBox::StandardButton reply = QMessageBox::question(
                    parentWindow, "Take Snapshot",
                            "Do you really want to take the snapshot\n \""
                                + snapName + "\" for the VM \"" + vmName +"\"",
                                            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return QStringList();
        }
    }

    // *** Проверка прав записи в каталоги хранения дисков *** //
    QStringList noWriteDirs;
    bool isWrite = checkWriteAccessToDirs(workDisks, &noWriteDirs);

    if (!isWrite){
        QMessageBox::critical(parentWindow, "Snapshot operation error…",
            "Unable to create file. "
                "Please check your write access:\n" + noWriteDirs.join("\n"));
        return QStringList();
    }

    // id - число секунд с 1970-ого года
    QString id = QString::number(QDateTime::currentSecsSinceEpoch());

    // Создание массива имён снапшотов
    QStringList snapshotsFullNames;
    for (int i = 0; i < workDisks.size(); ++i){
        snapshotsFullNames.push_back(getSnapName(workDisks[i], id));
    }

    // *** Проверка наличия разрешения на чтение qcow2 файлов, от которых *** //
    //     будет строиться новый снапшот. Если прав на чтение нет, то         //
    //     новый снапшот будет создан через virsh, который доступ к файлам    //
    //     иметь точно будет. При наличии прав на чтение, снапшоты будут      //
    //     созданы классическим образом через утилиту qemu-img                //
    bool allWorkDiskImagesReadable = checkAllImagesReadable(workDisks);

    if (allWorkDiskImagesReadable){
        doSnapshotOverQemuImg(vmName, workDisks, snapshotsFullNames);
    }
    else {
        doSnapshotOverVirsh(vmName, workDisks, snapshotsFullNames);
    }


    return snapshotsFullNames;
}

QStringList SnapManager::doSnapshotOverQemuImg(const QString& vmName,
                                         const QStringList& workDisks,
                                         const QStringList& snapshotsFullNames){
    // *** Английский язык в выводе утилиты qemu-img *** //
    QProcessEnvironment env;
    env.insert("LANG", "C");

    // *** Когда число примонтированных дисков значительно, *** //
    //     например, несколько десятков, то процесс создания    //
    //     снимка занимает ощутимое время.                      //
    //     Предлагается отображать ход создания снимка.         //
    QProgressDialog* progress = nullptr;
    progress = createNewQProgressDialog(snapshotsFullNames.size(),parentWindow);
    progress->show();

    // Индексы у workDisks и snapshotsFullNames согласованы
    for (int n = 0; n < snapshotsFullNames.size(); ++n){
        QEventLoop loop;
        QProcess process;
        process.setProcessEnvironment(env);

        QObject::connect(&process, &QProcess::finished, &loop,
                                                             &QEventLoop::quit);

        QStringList qemuImgArguments = {
                                            "create",
                                            "-f",
                                            "qcow2",
                                            "-b", workDisks[n],
                                            "-F",
                                            "qcow2",
                                            snapshotsFullNames[n]
                                        };

        process.start("qemu-img", qemuImgArguments);

        loop.exec();
        progress->setValue(n + 1);
    }

    // Смена точки монтирования в VM
    switchVmMountStorages(vmName, snapshotsFullNames);

    // *** Закрытие диалогового окна с прогрессом создания снапшота *** //
    //     Перед закрытием диалог повисит со 100%, чтобы не было        //
    //     лишнего "мельтешения" в случае короткой по времени операции  //
    this->sleep(1250);
    progress->close();
    progress->deleteLater();

    return snapshotsFullNames;
}

QStringList SnapManager::doSnapshotOverVirsh(const QString& vmName,
                                         const QStringList& workDisks,
                                         const QStringList& snapshotsFullNames){
    // *** Для сохранения возможности отображать прогресс создания *** //
    //     снапшотов, последние созаются отдельно для каждого диска.   //

    // Общая для всех снапшотов часть аргементов утилиты virsh
    QString virshArgsFirst = " snapshot-create-as " + vmName
                            + " snapFakeName-id-1234 --disk-only --no-metadata";

    // Заготовка для аргументов дисков (для всех дисков отключены снапшоты)
    QString diskspecNoSnaps;
    for (int i = 0; i < workDisks.size(); ++i){
        diskspecNoSnaps+= " --diskspec " + workDisks[i] + ",snapshot=no";
    }

    // *** Когда число примонтированных дисков значительно, *** //
    //     например, несколько десятков, то процесс создания    //
    //     снимка занимает ощутимое время.                      //
    //     Предлагается отображать ход создания снимка.         //
    QProgressDialog* progress = nullptr;
    progress = createNewQProgressDialog(snapshotsFullNames.size(),parentWindow);
    progress->show();

    // Включение создания снапшота для i-ого диска и формирование общей
    // строки аргементов утилиты virsh для создания снапшота i-ого диска
    //
    // *** Важно! Снапшоты, созданные через virsh предоставляют обычным *** //
    //     пользователям право на чтение снапшотов, что и позволяет их      //
    //     использовать для дальнейшего построения цепочки сохранений.      //
    //     Qcow2 файлы, подключенные к ВМ через VirtManager не дают доступа //
    //     обычным пользователям совсем (нет даже доступа к чтению файлов)  //
    for (int i = 0; i < workDisks.size(); ++i){
        QString diskspecDoSnaps = diskspecNoSnaps;

        // *** Включение снапшота для i-ого диска
        diskspecDoSnaps.replace(workDisks[i] + ",snapshot=no",
                               workDisks[i] + ",file=" + snapshotsFullNames[i]);
        QString virshArgs = virshArgsFirst + diskspecDoSnaps;

        QEventLoop loop;
        QProcess process;

        QObject::connect(&process, &QProcess::finished, &loop,
                                                             &QEventLoop::quit);

        process.start("virsh", {"--connect=" + m_libVirtConnectURI, virshArgs});

        loop.exec();
        progress->setValue(i + 1);

        // *** После i-ого снапшота в виртуальной машине будет подключен *** //
        //     не workDisks[i], а snapshotsFullNames[i]. Это надо учесть     //
        diskspecNoSnaps.replace(workDisks[i], snapshotsFullNames[i]);
    }

    // *** После установки в /etc/libvirt/qemu.conf параметра             *** //
    //     user = <username> при запуске ВМ подкюченный диск и вся его        //
    //     цепочка backing файлов получает владельца <username>.              //
    //     При выключении ВМ, примонтированные файлы возвращают своего        //
    //     владельца root:root, а владелец всей активной при включении ВМ     //
    //     цепочки сохранения так и остаётся за <username>:libvirt-qemu.      //
    //     Таким образом кратковременное включение ВМ позволит "сбросить"     //
    //     владельца до этого не читаемого файла на <username>:libvirt-qemu   //
    //     и получить право чтения и записи в него                            //
    this->turnOnOffVirtualMachine(vmName);

    // *** Закрытие диалогового окна с прогрессом создания снапшота *** //
    //     Перед закрытием диалог повисит со 100%, чтобы не было        //
    //     лишнего "мельтешения" в случае короткой по времени операции  //
    this->sleep(1250);
    progress->close();
    progress->deleteLater();

    return snapshotsFullNames;
}

bool SnapManager::checkAllImagesReadable(const QStringList& workDisks){
    bool res = true;

    for (int i = 0; i < workDisks.size(); ++i){
        QFile f(workDisks[i]);
        if (f.open(QIODevice::ReadOnly)){
            f.close();
        }
        else {
            res = false;
            break;
        }
    }

    return res;
}

void SnapManager::turnOnOffVirtualMachine(const QString& vmName){
    QEventLoop loop;
    QProcess process;

    QObject::connect(&process, &QProcess::finished, &loop, &QEventLoop::quit);

    // *** При старте ВМ и активации её сетевых интерфейсов в некоторых *** //
    //     системах, например, с NetworkManager'ом появляется уведомление   //
    //     об активности сетевых интерфейсов, что совсем ни к чему.         //
    //     Решение: удалить сетевые интерфейсы перед turnOnOff,             //
    //     а затем вернуть их на место.                                     //
    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                            "dumpxml", vmName});
    loop.exec();

    QString originalVmXml = process.readAllStandardOutput();
    QString noNetworkVmXml = removeVmXmlNetwork(originalVmXml);

    // *** Сохранение временного xml файла c обновлёнными настройками *** //
    QString noNetFileName = "/tmp/" + vmName + "-without-net.xml";
    QFile noNetFile(noNetFileName);
    if (noNetFile.open(QIODevice::WriteOnly | QIODevice::Text
                                                       | QIODevice::Truncate)){
        QTextStream out(&noNetFile);
        out << noNetworkVmXml;
        noNetFile.close();
    }

    // *** Применение новых настроек через virsh и удаление xml файла *** //
    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                      "define", noNetFileName});
    loop.exec();
    if (QFile::exists(noNetFileName)) {
        if (!QFile::remove(noNetFileName)) {
            qDebug() << "[EE] don't delete:" << noNetFileName;
        }
    }

    // *** Старт VM без сети *** //
    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                              "start", vmName});
    loop.exec();

    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                            "destroy", vmName});
    loop.exec();

    // // *** Восстановление оригинальной версии VM с сетью *** //
    QString originalVmFileName = "/tmp/" + vmName + ".xml";
    QFile originalVmFile(originalVmFileName);
    if (originalVmFile.open(QIODevice::WriteOnly | QIODevice::Text
                                                        | QIODevice::Truncate)){
        QTextStream out(&originalVmFile);
        out << originalVmXml;
        originalVmFile.close();
    }
    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                 "define", originalVmFileName});
    loop.exec();
    if (QFile::exists(originalVmFileName)) {
        if (!QFile::remove(originalVmFileName)) {
            qDebug() << "[EE] don't delete:" << originalVmFileName;
        }
    }
}

QString SnapManager::removeVmXmlNetwork(const QString& vmXml){
    QDomDocument doc;
    doc.setContent(vmXml);

    QDomElement root = doc.documentElement();
    QDomNodeList interfaces = root.elementsByTagName("interface");

    for (int i = interfaces.count() - 1; i >= 0; --i) {
        QDomNode node = interfaces.at(i);
        QDomElement elem = node.toElement();
        if (!elem.isNull() && elem.attribute("type") == "network") {
            node.parentNode().removeChild(node);
        }
    }

    return doc.toString(4);
}

void SnapManager::detachBlockDevice(const QString& vmName,
                                                        const QString& devName){
    QProcess process;
    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                               "detach-disk", vmName, devName, "--persistent"});

    if (!process.waitForFinished()){
        return;
    }
}

void SnapManager::mountBlockDevice(const QString& vmName,
                                                const QString& imageFullName,
                                                    const QString& blockDevice,
                                                        const QString& busType){
    QProcess process;
    process.start("virsh", {"--connect=" + m_libVirtConnectURI, "attach-disk",
                            "--domain="  + vmName,
                            "--source="  + imageFullName,
                            "--target="  + blockDevice,
                            "--targetbus=" + busType,
                            "--subdriver=qcow2", "--persistent"} );

    if (!process.waitForFinished()){
        return;
    }
}

void SnapManager::createQcow2Images(const QVector<ChainNode>& vmNodes,
                      const QString& newImageSize, const int& insertDriveIndex){
    // *** Когда число сохранённых состояний (cнапшотов) значительно,  *** //
    //     например, несколько десятков, то процесс дополения всех снимков //
    //     состояний до полного комплекта занимает ощутимое время.         //
    //     Предлагается отображать ход дополнения снапшотов.               //
    QProgressDialog* progress = nullptr;
    progress = createNewQProgressDialog(vmNodes.size(),parentWindow);
    progress->show();

    // -> цикл по всем узлам c определением пары imageFile & backingFile
    for (int i = 0; i < vmNodes.size(); ++i){
        QString imageFile = vmNodes[i].imagesFullNames[insertDriveIndex];
        QString backingFile = vmNodes[i].backFullNames[insertDriveIndex];
        this->createQcow2Image(imageFile, backingFile, newImageSize);
        progress->setValue(i+1);
    }

    // *** Закрытие диалогового окна с прогрессом операции *** //
    //     Перед закрытием диалог повисит со 100%, чтобы       //
    //     не было "мельтешения" в случае быстрой операции     //
    this->sleep(1250);
    progress->close();
    progress->deleteLater();
}

void SnapManager::createQcow2Image(const QString& imageFile,
                              const QString& backingFile, const QString& iSize){
    QProcess process;
    QEventLoop loop;

    QObject::connect(&process, &QProcess::finished, &loop,
                                                             &QEventLoop::quit);

    if (backingFile == "None"){
        process.start("qemu-img", {"create", "-f", "qcow2",
                                                       imageFile, iSize + "G"});
    }
    else {
        process.start("qemu-img", {"create", "-f", "qcow2", "-b", backingFile,
                                                     "-F", "qcow2", imageFile});
    }

    loop.exec();
}

QStringList SnapManager::gotoSnapshot(const QString& vmName,
                                                         const ChainNode& node){
    QStringList res;

    if (node.imagesType == "work"){
        // *** Для случая "work" достаточно изменить <source></source> *** //
        switchVmMountStorages(vmName, node.imagesFullNames);
        res = node.imagesFullNames;
    }
    else {
        // *** В случае "snap" сначала необходимо сделать "work" снапшот *** //
        //     Режим 'silence', поэтому имя узла не требуется "fakeSnapName" //
        res = doSnapshot(vmName, "fakeSnapName", node.imagesFullNames, true);
    }

    // *** Фактически возвращения списка подключенных к ВМ хранилищ *** //
    return res;
}

bool SnapManager::deleteSnapshot(const QString& vmName, const QString& snapName,
                                                         const ChainNode& node){

    if ( node.parentId == -1 ){
        if (node.childrenImagesFullNames.size() > 1 ) {
            QMessageBox::information(parentWindow,"Root chain node deletion…",
                "Info: The root snapshot can only be removed with one child.");
                return false;
        }
        else {
            // *** Delete confirmation in this specific case (one child)  *** //
            QMessageBox::StandardButton reply = QMessageBox::question(
                                parentWindow, "Delete confirmation",
                                "Do you really want to delete the snapshot "
                                                  "\"" + snapName + "\"?\n\n"
                        "Info: Deleting root snapshot may take a long time \n"
                               "and temporarily require additional disk space.",
                                            QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) {
                return false;
            }

            // Есть корневой узел с одним потомком. Удаление корневого узла,
            // есть конвертация потомка в независимый диск, который станет новым
            // корневым узлом цепочки сохранения состояний
            QStringList idImages = node.imagesFullNames;
            QStringList childImages = node.childrenImagesFullNames.first();
            doNewRoot(idImages, childImages);

            return true;
        }
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
                                parentWindow, "Delete confirmation",
                                "Do you really want to delete the snapshot "
                                                     "\"" + snapName + "\"?\n",
                                            QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return false;
    }

    // *** Проверка записи в каталоги, а значит и удаления из них *** //
    QStringList noWriteDirs;
    bool isWrite = checkWriteAccessToDirs(node.imagesFullNames, &noWriteDirs);
    if (!isWrite){
        QMessageBox::critical(parentWindow, "Snapshot operation error",
            "Unable to delete file. "
                "Please check your write access:\n" + noWriteDirs.join("\n"));
        return false;
    }

    if (node.imagesType == "work"){
        // Для случая "work" + не активное состояние необходимо
        // просто удалить соответствующие диски ВМ.
        for (int i = 0; i < node.imagesFullNames.size(); ++i){
            QFile file(node.imagesFullNames[i]);
            if (file.exists()){
               file.remove();
            }
        }
        return true;
    }
    else {
        if (node.imagesType == "snap"){
            // Остаётся случай "snap" у которого есть
            // a) один родитель (или родителя нет в случае корневого узла)
            // б) один или более потомков(по размеру childrenImagesFullNames

            // Задача перебазировать все жёсткие диски потомков (children)
            // с текущего места (id) на родителя (parentId) текущего узла
            QStringList parentImages = node.backFullNames;
            QStringList idImages = node.imagesFullNames;
            QVector<QStringList> childrenImages = node.childrenImagesFullNames;
            return rebaseImages(parentImages, idImages, childrenImages);
        }
    }

    return false;
}

bool SnapManager::deleteImageFiles(const QStringList& rmImagesList){
    bool res = true;

    for (int i = 0; i < rmImagesList.size(); ++i) {
        QString path = rmImagesList[i];
        if (!QFile::exists(path) || !QFile::remove(path))
            res = false;
    }

    return res;
}

QString SnapManager::getSnapName(const QString& imgName, const QString& id){
    QString snapName = imgName;

    if (snapName.endsWith(".qcow2", Qt::CaseInsensitive)) {
        snapName.chop(6);
    }

    const static QRegularExpression pattern1(R"((-id-\d{10}))");
    snapName = snapName.replace(pattern1, "");
    snapName += "-id-" + id + ".qcow2";
    return snapName;
}

void SnapManager::switchVmMountStorages(const QString& vmName,
                                               const QStringList& snapStorages){
    // *** Получение vmXmlDoc *** //
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");

    QProcess process;
    process.setProcessEnvironment(env);
    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                            "dumpxml", vmName});

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
        qDebug() << "[EE] Failed to write back XML file.";
        return;
    }

    QTextStream out(&file);
    vmXmlDoc.save(out, 4);
    file.close();

    // *** Применение новых настроек через virsh *** //
    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                           "define", fileName});

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

bool SnapManager::rebaseImages(const QStringList& parentImages,
    const QStringList& idImages, const QVector<QStringList>& childrenImages){

    int totalChildren = childrenImages.size();
    int childImages = childrenImages[0].size();
    int totalFiles = totalChildren * childImages;

    // *** Проверка доступности файлов потомков для записи *** //
    for (int i = 0; i < totalChildren; ++i){ // Node loop
        for (int j = 0; j < childImages; ++j){ // Images loop
                QFileInfo file(childrenImages[i][j]);
                if (!file.isWritable()) {
                    QMessageBox::critical(parentWindow, "Permission error",
                       "Error: The file must be writable:\n" + file.fileName());
                    return false;
                }
        }
    }

    QProgressDialog* progress = nullptr;
    progress = createNewQProgressDialog(100*totalFiles, parentWindow);
    progress->setAutoClose(false);
    progress->setAutoReset(false);
    progress->show();

    QString partInfo, info;
    int doneFiles = 0;

    // Обновление процента обработки текущего файла в реальном времени
    for (int i = 0; i < totalChildren; ++i){ // Node loop
        for (int j = 0; j < childImages; ++j){ // Images loop

            // *** Вывод информации во всплывающее окно *** //
            partInfo = QString("Updating snapshot structure\n DANGER: "
            "Manually canceling this process may corrupt or cause data loss!\n"
                                    "Please wait…\n"
                            "Processing file %1 of %2 (%4%) \n '%3'")
                               .arg(doneFiles+1, 3)
                               .arg(totalFiles,  3)
                               .arg(QFileInfo(childrenImages[i][j]).fileName());
            info = partInfo.arg(0, 3, 10, QChar('0'));
            progress->setLabelText(info);

            QTimer rebaseFileProgressTimer;
            rebaseFileProgressTimer.setInterval(500);

            QProcess process;
            QEventLoop loop;

            long int rebaseDataValue = getRebaseDataValue(idImages[j],
                                                          childrenImages[i][j]);
            long int fSizeOld = QFileInfo(childrenImages[i][j]).size();

            // *** Обновление по таймеру актуально для большого файла, *** //
            //    когда ребазирование происходит длительное время. При     //
            //    небольших файлах таймер убивается раньше, чем сработает  //
            QObject::connect(&rebaseFileProgressTimer, &QTimer::timeout,[&](){
                    long int fSize = QFileInfo(childrenImages[i][j]).size();
                    int value = qRound(100.0*(fSize-fSizeOld)/rebaseDataValue);
                    info = partInfo.arg(value, 3, 10, QChar('0'));
                    progress->setLabelText(info);
                    progress->setValue(100*doneFiles + value);
                });

            QObject::connect(&process, &QProcess::finished, &loop,
                                                             &QEventLoop::quit);

            QStringList qemuArgs = {"rebase","-p", "-f", "qcow2", "-F", "qcow2",
                                   "-b", parentImages[j], childrenImages[i][j]};

            process.start("qemu-img", qemuArgs);
            rebaseFileProgressTimer.start();
            loop.exec();
            rebaseFileProgressTimer.stop();

            doneFiles++;
            progress->setValue(100*doneFiles);
        }
    }

    // *** Really delelte snapshot files *** //
    for (int k = 0; k < idImages.size(); ++k){
        QFile file(idImages[k]);
        if (file.exists()){
            file.remove();
        }
    }

    // *** Задержать закрытие прогресса при выполнении работы *** //
    //        Отображение 100% индикатора в течение 1,25 с
    info = partInfo.arg(100, 3, 10, QChar('0'));
    progress->setLabelText(info);
    this->sleep(1250);

    progress->close();
    progress->deleteLater();
    return true;
}

void SnapManager::doNewRoot(const QStringList& idImgs,
                                                const QStringList& childImgs){
    QProgressDialog* progress = nullptr;
    progress = createNewQProgressDialog(100*childImgs.size(), parentWindow);
    progress->setAutoClose(false);
    progress->setAutoReset(false);
    progress->show();
    QString partInfo, info;

    for (int i = 0; i < idImgs.size(); ++i){
        // *** Вывод информации во всплывающее окно *** //
        partInfo = QString("Moving required data to the new snapshot tree root\n"
            "DANGER: Manually canceling this process may corrupt or cause data "
            "loss!\n Please wait…\n"
                            "Processing file %1 of %2 (%4%) \n '%3'")
                               .arg(i+1, 3)
                               .arg(idImgs.size(),  3)
                               .arg(QFileInfo(childImgs[i]).fileName());
        info = partInfo.arg(0, 3, 10, QChar('0'));
        progress->setLabelText(info);

        QProcess process;
        QEventLoop loop;

        QObject::connect(&process, &QProcess::finished, &loop,
                                                             &QEventLoop::quit);

        QObject::connect(&process, &QProcess::readyReadStandardOutput,
            [&](){
                QString output = process.readAllStandardOutput();

                // (12.01/100%)
                QRegularExpression re(R"(\((\d+(?:\.\d+)?)/(\d+)%\))");
                QRegularExpressionMatch match = re.match(output);

                if (match.hasMatch()) {
                    QString valueStr = match.captured(1); // "12.01"
                    int value  = qRound(valueStr.toFloat());
                    info = partInfo.arg(value, 3, 10, QChar('0'));
                    progress->setLabelText(info);
                    progress->setValue(100*i + value);
                }
            });

        QStringList qemuArgs = {"convert", "-O", "qcow2", "-p",
                              childImgs[i], childImgs[i] + ".temp-copy.qcow2" };

        process.start("qemu-img", qemuArgs);

        loop.exec();
    }

    // *** Really move snapshot files *** //
    for (int k = 0; k < idImgs.size(); ++k){
        // Delete old root node
        QFile file(idImgs[k]);
        if (file.exists()){
            file.remove();
        }

        // Delete old child
        QFile fileOldChildImg(childImgs[k]);
        if (fileOldChildImg.exists()){
            fileOldChildImg.remove();
        }

        // Rename temp to root
        QFile::rename(childImgs[k] + ".temp-copy.qcow2", childImgs[k]);
    }

    progress->setValue(childImgs.size()*100);

    // *** Задержка с отобржаением 100% прогресса операции *** //
    this->sleep(1250);
    progress->close();
    progress->deleteLater();
}

bool SnapManager::checkWriteAccessToDirs(const QStringList& dirsForWriteCheck,
                                                      QStringList* noWriteDirs){
    bool writeFlag = true;

    // *** Формирование уникального списка каталогов для проверки записи *** //
    QStringList checkDirs;
    QSet<QString> uniqueSet;
    for (int i = 0; i < dirsForWriteCheck.size(); ++i){
        uniqueSet.insert(QFileInfo(dirsForWriteCheck[i]).absolutePath());
    }
    checkDirs = uniqueSet.values();

    // *** Проверка на запись/удаление путём создания временного файла *** //
    for (int i = 1; i < checkDirs.size(); ++i){
        QString writeTestPath = checkDirs[i];
        QTemporaryFile createTestFile(writeTestPath + "/.writeTestFile-XXXXXX");
        if (createTestFile.open()){
            createTestFile.remove();
        }
        else {
            writeFlag = false;
            noWriteDirs->push_back(checkDirs[i]);
        }
    }

    return writeFlag;
}

QProgressDialog* SnapManager::createNewQProgressDialog(const int& maxValue,
                                                        QWidget* parentWindow){
    QProgressDialog* progress = new QProgressDialog("", "", -1,
                                                        maxValue, parentWindow);
    progress->setWindowTitle("Operation progress");
    progress->setCancelButton(nullptr);
    progress->setWindowFlags(Qt::Dialog
                            | Qt::WindowTitleHint
                            | Qt::CustomizeWindowHint);
    progress->setMinimumWidth(445);
    progress->setWindowModality(Qt::WindowModal);
    progress->setAutoClose(false);
    progress->setAutoReset(false);
    // *** Окно иногда появляется не в центре родительского окна *** //
    //        Возможно, принудительное задание позиции поможет       //
    //         (Интересно, как оно будет работать в Wayland)         //
    QRect parentGeom = parentWindow->geometry();
    QSize dlgSize = progress->size();
    int x = parentGeom.x() + (parentGeom.width() - dlgSize.width()) / 2;
    int y = parentGeom.y() + (parentGeom.height() - dlgSize.height()) / 2 - 60;
    progress->move(x, y);
    // ************************************************************* //
    return progress;
}

long int SnapManager::getRebaseDataValue(const QString& backFullName,
                                                    const QString& rebaseImage){
    long int resBlockData = 0;
    QString backingFile = QFile(backFullName).fileName();

    // *** Через qemu-img map <rebaseImage> идёт сбор числа блоков *** //
    //     данных расположеных в родительском файле. Для каждого блока //
    //     вычисляется размер и по общему размеру определяется размер  //
    //     данных для rebasing'а                                       //
    // *************************************************************** //

    QProcess process;
    QEventLoop loop;

    QObject::connect(&process, &QProcess::finished, [&](){
            QString output = process.readAllStandardOutput();
                QStringList outputLines = output.split('\n',Qt::SkipEmptyParts);
                for (int i = 0; i < outputLines.size(); ++i){
                    if (i > 1){
                        QString line = outputLines[i];
                        QStringList parts = line.simplified().split(" ");
                        if (parts.size() >=4 && line.contains(backingFile)){
                            QString hexDataSize = parts.at(1);
                            bool ok;
                            long int blockData = hexDataSize.toLong(&ok, 16);
                            if (ok){
                                resBlockData+=blockData;
                            }
                        }
                    }
                }
                loop.quit();
            });

    process.start("qemu-img", {"map", rebaseImage});
    loop.exec();

    return resBlockData;
}

void SnapManager::sleep(const int& msTimer){
    QEventLoop dlgLoop;
    QTimer::singleShot(msTimer, &dlgLoop, &QEventLoop::quit);
    dlgLoop.exec();
}
// End snapManager.cpp
