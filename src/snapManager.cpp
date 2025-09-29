// Begin snapManager.cpp

#include "snapManager.h"

SnapManager::SnapManager(QWidget* parent) : parentWindow(parent){
}

SnapManager::~SnapManager(){
}

QStringList SnapManager::doSnapshot(const QString& name,
                                    const QStringList& workDisks, bool silence){
    // *** Подтверждение создания снапшота *** //
    if (!silence) {
        QMessageBox::StandardButton reply = QMessageBox::question(
                    parentWindow, "Take Snapshot - " + name,
                        "Do you really want to take a snapshot of '"+ name +"'",
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

    // // Создание и запуск внешней команды
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");

    // Индексы у workDisks и snapshotsFullNames согласованы
    for (int n = 0; n < snapshotsFullNames.size(); ++n){
        QEventLoop loop;
        QProcess process;
        process.setProcessEnvironment(env);

        QObject::connect(&process, &QProcess::finished, &loop,
                                                             &QEventLoop::quit);

        QString snapName = snapshotsFullNames[n];
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
    }

    // Смена точки монтирования в VM
    switchVmMountStorages(name, snapshotsFullNames);
    return snapshotsFullNames;
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
        res = doSnapshot(vmName, node.imagesFullNames, true);
    }

    // *** Фактически возвращения списка подключенных к ВМ хранилищ *** //
    return res;
}

bool SnapManager::deleteSnapshot(const QString& vmName, const ChainNode& node){

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
                                "Do you really want to delete the snapshot?\n\n"
                       "Deleting root snapshot may take a long time \n"
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
                                   "Do you really want to delete the snapshot?",
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

QString SnapManager::getSnapName(const QString& imgName, const QString& id){
    QString snapName = imgName;

    if (snapName.endsWith(".qcow2", Qt::CaseInsensitive)) {
        snapName.chop(6);
    }

    QRegularExpression pattern1(R"((-id-\d{10}))");
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
        qDebug() << "[EE] Failed to write back XML file.";
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
                    qDebug() << "[EE] Error: The file must be writable"
                                                        << childrenImages[i][j];
                    QMessageBox::critical(parentWindow, "Permission error…",
                       "Error: The file must be writable:\n" + file.fileName());
                    return false;
                }
        }
    }

    QProgressDialog* progress = nullptr;
    progress = createNewQProgressDialog(100*totalFiles, parentWindow);
    progress->show();
    // QApplication::processEvents();
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

            // *** Debug rebase *** //
            // QEventLoop loopT;
            // QTimer::singleShot(500, &loopT, &QEventLoop::quit);
            // loopT.exec();

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
                        progress->setValue(100*doneFiles + value);
                    }
                });

            QStringList qemuArgs = {"rebase","-p", "-f", "qcow2", "-F", "qcow2",
                                   "-b", parentImages[j], childrenImages[i][j]};

            process.start("qemu-img", qemuArgs);

            loop.exec();

            doneFiles++;
        }
    }

    // *** Really delelte snapshot files *** //
    for (int k = 0; k < idImages.size(); ++k){
        QFile file(idImages[k]);
        if (file.exists()){
            file.remove();
        }
    }
    progress->setValue(totalFiles);
    progress->close();
    progress->deleteLater();

    return true;
}

void SnapManager::doNewRoot(const QStringList& idImgs,
                                                const QStringList& childImgs){
    QProgressDialog* progress = nullptr;
    progress = createNewQProgressDialog(100*childImgs.size(), parentWindow);
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
    progress->setWindowTitle("Data transfer");
    progress->setCancelButton(nullptr);
    progress->setWindowFlags(Qt::Dialog
                            | Qt::WindowTitleHint
                            | Qt::CustomizeWindowHint);
    progress->setMinimumWidth(445);
    progress->setWindowModality(Qt::WindowModal);
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
// End snapManager.cpp
