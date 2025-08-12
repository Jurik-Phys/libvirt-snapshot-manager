// Begin appWindow.cpp

#include "appWindow.h"

QAppWindow::QAppWindow(QWidget *parent) : QWidget(parent){
    this->resize(m_appWindowWidth, m_appWindowHeight);
    this->setWindowTitle("Graphical Manager for External Snapshots (libvirt)");

    QHBoxLayout* hAppLayout = new QHBoxLayout(this);

    m_vLColumnLayout = new QVBoxLayout();
    m_vRColumnLayout = new QVBoxLayout();

    hAppLayout->addLayout(m_vLColumnLayout);
    hAppLayout->addLayout(m_vRColumnLayout);

    m_vVmLayout = new QVBoxLayout;
    m_vVmLayout->setAlignment(Qt::AlignTop);
    m_vSnapLayout = new QVBoxLayout;

    // VmDataCollector vmDataCollector;
    // m_vmList = vmDataCollector.getVmList();
    VmDataCollector* vmDataCollector = new VmDataCollector();
    m_vmList = vmDataCollector->getVmList();

    QThread* getVmListThread = new QThread();
    vmDataCollector->moveToThread(getVmListThread);
    QObject::connect(getVmListThread, &QThread::started,
                           vmDataCollector, &VmDataCollector::vmListStartTimer);
    QObject::connect(vmDataCollector, &VmDataCollector::vmListReady,
                                              this, &QAppWindow::onVmListReady);
    QObject::connect(this, &QAppWindow::vmListProcessingStarted,
                            vmDataCollector, &VmDataCollector::vmListStopTimer);
    QObject::connect(this, &QAppWindow::vmListProcessingCompleted,
                           vmDataCollector, &VmDataCollector::vmListStartTimer);

    getVmListThread->start();

    m_snapTreeModel = new SnapTreeModel();
    m_snapTreeLoadingModel = new SnapTreeModel();
    ChainNode loadingNode = {1, -1,
                          "Loading snapshot chain information. Please wait …"};
    m_snapTreeLoadingModel->setSnapData({loadingNode});

    setVmBtnFrame();
    setVmFrame();
    setSnapBtnFrame();
    setSnapFrame();

    addVmToFrame();
}

QAppWindow::~QAppWindow(){
}

void QAppWindow::appExit(){
   QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Exit confirmation", "Do you really want to exit?",
                                            QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
}

void QAppWindow::setVmBtnFrame(){
    QFrame* vmBtnFrame = new QFrame(this);
    vmBtnFrame->setFrameShape(QFrame::StyledPanel);
    vmBtnFrame->setFrameShadow(QFrame::Plain);
    vmBtnFrame->setFixedHeight(m_headFrameHeight);
    vmBtnFrame->setFixedWidth(m_appWindowWidth/2.5);

    QHBoxLayout* vmBtnFrameHLayout = new QHBoxLayout(vmBtnFrame);

    m_startBtn = new QToolButton(vmBtnFrame);
    m_startBtn->setText("Start");
    m_startBtn->setFixedHeight(m_btnSize1);
    m_startBtn->setFixedWidth(1.5*m_btnSize1);
    m_startBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    QObject::connect(m_startBtn, &QToolButton::clicked,
                                                    this, &QAppWindow::startVM);
    m_startBtn->setEnabled(false);

    QToolButton* stopBtn = new QToolButton(vmBtnFrame);
    stopBtn->setText("Stop");
    stopBtn->setFixedHeight(m_btnSize1);
    stopBtn->setFixedWidth(1.5*m_btnSize1);
    stopBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    m_takeSnapBtn = new QToolButton(vmBtnFrame);
    m_takeSnapBtn->setText("Take");
    m_takeSnapBtn->setFixedHeight(m_btnSize1);
    m_takeSnapBtn->setFixedWidth(1.5*m_btnSize1);
    m_takeSnapBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_takeSnapBtn->setEnabled(false);
    QObject::connect(m_takeSnapBtn, &QToolButton::clicked, this,
                                                       &QAppWindow::doSnapshot);

    vmBtnFrameHLayout->addWidget(m_startBtn);
    vmBtnFrameHLayout->addWidget(stopBtn);
    vmBtnFrameHLayout->addWidget(m_takeSnapBtn);

    m_vLColumnLayout->addWidget(vmBtnFrame);
}

void QAppWindow::setVmFrame(){
    QScrollArea* vmFrame = new QScrollArea(this);
    vmFrame->setWidgetResizable(true);

    QWidget* container = new QWidget(vmFrame);
    container->setLayout(m_vVmLayout);
    vmFrame->setWidget(container);

    vmFrame->setStyleSheet("background-color: white;");
    vmFrame->setFixedWidth(m_appWindowWidth/2.5);

    m_vLColumnLayout->addWidget(vmFrame);
}

void QAppWindow::setSnapBtnFrame(){
    QFrame* snapBtnFrame = new QFrame(this);
    snapBtnFrame->setFrameShape(QFrame::StyledPanel);
    snapBtnFrame->setFrameShadow(QFrame::Plain);
    snapBtnFrame->setFixedHeight(m_headFrameHeight);

    QHBoxLayout* snapBtnFrameHLayout = new QHBoxLayout(snapBtnFrame);

    m_gotoBtn = new QToolButton(snapBtnFrame);
    m_gotoBtn->setText("Go to");
    m_gotoBtn->setFixedHeight(m_btnSize1);
    m_gotoBtn->setFixedWidth(1.5*m_btnSize1);
    m_gotoBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_gotoBtn->setEnabled(false);
    QObject::connect(m_gotoBtn, &QToolButton::clicked, this,
                                                     &QAppWindow::gotoSnapshot);

    m_deleteBtn = new QToolButton(snapBtnFrame);
    m_deleteBtn->setText("Delete");
    m_deleteBtn->setFixedHeight(m_btnSize1);
    m_deleteBtn->setFixedWidth(1.5*m_btnSize1);
    m_deleteBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_deleteBtn->setEnabled(false);
    QObject::connect(m_deleteBtn, &QToolButton::clicked, this,
                                                   &QAppWindow::deleteSnapshot);

    QToolButton* exitBtn = new QToolButton(snapBtnFrame);
    exitBtn->setText("Exit");
    exitBtn->setFixedHeight(m_btnSize1);
    exitBtn->setFixedWidth(1.5*m_btnSize1);
    exitBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    QObject::connect(exitBtn, &QToolButton::clicked, this,&QAppWindow::appExit);

    snapBtnFrameHLayout->addWidget(m_gotoBtn);
    snapBtnFrameHLayout->addWidget(m_deleteBtn);
    snapBtnFrameHLayout->addStretch();
    snapBtnFrameHLayout->addWidget(exitBtn);

    m_vRColumnLayout->addWidget(snapBtnFrame);
}

void QAppWindow::setSnapFrame(){

    m_snapTreeView = new SnapTreeView();
    m_snapTreeView->header()->setStretchLastSection(false);
    m_snapTreeView->header()
                          ->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_snapTreeView->setAlternatingRowColors(true);
    m_snapTreeView->header()->hide();
    m_snapTreeView->setStyleSheet(R"(
                                        QTreeView::item {
                                            height: 40px;
                                        }
                                    )");
    QFont font = m_snapTreeView->font();
    font.setPointSize(10);
    m_snapTreeView->setFont(font);
    // Установка кастомного стиля, для отображения веток дерева
    m_snapTreeView->setStyle(new TreeLinesStyle());

    m_vRColumnLayout->addWidget(m_snapTreeView);

    m_snapTreeView->setModel(m_snapTreeModel);

    // Установка кастомного делегата для item'ов
    m_snapTreeView->setItemDelegate(new TreeItemDelegate(m_snapTreeView));

    QObject::connect(m_snapTreeView, &SnapTreeView::clicked,
                                          this, &QAppWindow::onTreeItemClicked);

    QObject::connect(m_snapTreeView, &SnapTreeView::clicked,
                                              this, &QAppWindow::gotoAndDelBtnManage);
    QObject::connect(m_snapTreeView, &SnapTreeView::doSnapshot,
                                              this, &QAppWindow::doSnapshot);
    QObject::connect(m_snapTreeView, &SnapTreeView::deleteSnapshot,
                                             this, &QAppWindow::deleteSnapshot);
    QObject::connect(m_snapTreeView, &SnapTreeView::gotoSnapshot,
                                             this, &QAppWindow::gotoSnapshot);
    QObject::connect(m_snapTreeView, &SnapTreeView::startVM,
                                                    this, &QAppWindow::startVM);
}

void QAppWindow::addVmToFrame(){
    QStringList vmNameList;
    QVector<int> vmIdx;

    // *** Получение списка имён виртуальных машин *** //
    for (int k = 0; k < m_vmList.size(); ++k){
        vmNameList.push_back(m_vmList[k].name);
    }
    // *** Сортировка списка виртуальных машин *** //
    vmNameList.sort(Qt::CaseInsensitive);

    // *** Получение вектора индексов виртуальных машин *** //
    for (int i = 0; i < vmNameList.size(); ++i){
        for (int j = 0; j < m_vmList.size(); ++j){
            if (m_vmList[j].name == vmNameList[i]){
                vmIdx.push_back(j);
            }
        }
    }

    // *** Временный вектор отсортированных виртуальных машин *** //
    QVector<VMachine> tmpVmList;
    for (int i = 0; i < m_vmList.size(); ++i){
        tmpVmList.push_back(m_vmList[vmIdx[i]]);
    }

    // *** Список виртуальных машин отсортирован по алфавиту *** //
    m_vmList = tmpVmList;

    // *** Непосредственная вставка *** //
    for (int i = 0; i < m_vmList.size(); ++i){
        VmWidget* vmWidget = new VmWidget(m_vmList[i], this);
        m_vVmLayout->addWidget(vmWidget);
        QObject::connect(vmWidget, &VmWidget::clicked, this,
            [this, vmWidget](){
                int nMax = m_vVmLayout->count();
                for (int n = 0; n < nMax; ++n) {
                    QLayoutItem* item = m_vVmLayout->itemAt(n);
                    VmWidget* widget = static_cast<VmWidget*>(item->widget());
                    if (widget != vmWidget){
                        widget->setSelected(false);
                    }
                    else {
                        m_selectedVmIndex = n;
                    }
                }
            });
        QObject::connect(vmWidget, &VmWidget::clicked, this,
                                                      &QAppWindow::updSnapTree);
        QObject::connect(vmWidget, &VmWidget::clicked, this,
                                                   &QAppWindow::startBtnManage);
    }
}

void QAppWindow::addVmToFrame(const QVector<VMachine>& toAddvmList){
    for (int i = 0; i < toAddvmList.size(); ++i){
        // *** Индекс вставки VM *** //
        int indexWidget = getInsertWidgetIndex(m_vmList, toAddvmList[i]);
        // *** Новую VM надо добавить не только на форму, но и в список *** //
        m_vmList.insert(indexWidget, toAddvmList[i]);

        VmWidget* vmWidget = new VmWidget(toAddvmList[i], this);
        m_vVmLayout->insertWidget(indexWidget, vmWidget);
        QObject::connect(vmWidget, &VmWidget::clicked, this,
            [this, vmWidget](){
                int nMax = m_vVmLayout->count();
                for (int n = 0; n < nMax; ++n) {
                    QLayoutItem* item = m_vVmLayout->itemAt(n);
                    VmWidget* widget = static_cast<VmWidget*>(item->widget());
                    if (widget != vmWidget){
                        widget->setSelected(false);
                    }
                    else {
                        m_selectedVmIndex = n;
                    }
                }
            });
        QObject::connect(vmWidget, &VmWidget::clicked, this,
                                                     &QAppWindow::updSnapTree);
        QObject::connect(vmWidget, &VmWidget::clicked, this,
                                                  &QAppWindow::startBtnManage);
    }
}

void QAppWindow::delVmFromFrame(const QVector<VMachine>& toDelVmList){
    for (int i = 0; i < toDelVmList.size(); ++i){
        // *** Индексы в m_vmList и в m_vVmLayout совпадают *** //
        int rmIdx =getDeleteWidgetIndex(m_vmList, toDelVmList[i]);

        QLayoutItem* item = m_vVmLayout->itemAt(rmIdx);
        if (item) {
            QWidget* widget = item->widget();
            if (widget) {
                m_vVmLayout->removeWidget(widget);
                widget->deleteLater();
                // *** Удаление из списка виртуальных машин *** //
                m_vmList.removeAt(rmIdx);
            }
        }
    }
}

void QAppWindow::modVmIntoFrame(const QVector<VMachine>& toModVmList){
    for (int i = 0; i < toModVmList.size(); ++i){
        int modIdx = getModifyWidgetIndex(m_vmList, toModVmList[i]);
        QLayoutItem* item = m_vVmLayout->itemAt(modIdx);
        if (item) {
            VmWidget* widget = static_cast<VmWidget*>(item->widget());
            if (widget) {
                widget->setProperties(toModVmList[i]);

                // *** Изменение свойств в списке виртуальных машин *** //
                if (m_vmList[modIdx].name  != toModVmList[i].name){
                    // *** Изменяем имя виртуальной машины в списке машин *** //
                    m_vmList[modIdx].name  = toModVmList[i].name;

                    // *** Определение нового индекса (имя же изменилось) *** //
                    QStringList vmNameList;
                    for (int i = 0; i < m_vmList.size(); ++i){
                        vmNameList.push_back(m_vmList[i].name);
                    }
                    vmNameList.sort(Qt::CaseInsensitive);
                    int newModIdx = vmNameList.indexOf(toModVmList[i].name);

                    // *** Перемещение в списке виртуальных машин *** //
                    m_vmList.move(modIdx, newModIdx);

                    // *** Перемещение в виджетах *** //
                    m_vVmLayout->removeWidget(widget);
                    m_vVmLayout->insertWidget(newModIdx, widget);
                }
                // Обновление статуса в списке виртуальных машин;
                m_vmList[modIdx].state = toModVmList[i].state;
            }
        }
    }
}

void QAppWindow::updSnapTree(){
    QObject* s = sender();
    VmWidget* vmWidget = qobject_cast<VmWidget *>(s);
    if (vmWidget) {
        vmWidget->setLoadingFlag(true);
    }

    VMachine activeVm = getActiveVm();

    qDebug() << "[II] Update snap tree now!";
    qDebug() << "[II] Selected VM index:" << m_selectedVmIndex;

    m_snapTreeView->setModel(m_snapTreeLoadingModel);

    QThread* thread = new QThread;
    VmDataCollector* vmDataCollector = new VmDataCollector(activeVm, this);
    QObject::connect(vmDataCollector, &VmDataCollector::errorMsg, this,
                                                &QAppWindow::showErrorMessage);
    vmDataCollector->moveToThread(thread);

    thread->start();

    QObject::connect(thread, &QThread::started,
                                    vmDataCollector, &VmDataCollector::process);

    QObject::connect(vmDataCollector, &VmDataCollector::finished, this,
        [=](const VMachine& result) {
                QVector<ChainNode> vmSnapshotsChain = result.vmStateChain;
                m_snapTreeView->setModel(m_snapTreeModel);
                m_snapTreeView->clearSelection();
                m_snapTreeModel->setSnapData(vmSnapshotsChain);
                m_snapTreeView->expandAll();
                m_currentVmName = result.name;
                m_mountStorages = result.mountStorages;

                // for (int i = 0; i < result.vmStateChain.size(); ++i){
                //    qDebug() << vmSnapshotsChain[i].id
                //             << vmSnapshotsChain[i].parentId
                //             << vmSnapshotsChain[i].name;
                // }
                qDebug() << "[II] Данные получены (finished)";

                // *** On/Off GoTo button *** //
                gotoAndDelBtnManage();

                thread->quit();
                thread->wait();

                vmDataCollector->deleteLater();
                this->takeSnapBtnManage();
                thread->deleteLater();

                // *** Снятие флага продолжения загрузки *** //
                vmWidget->setLoadingFlag(false);
        });
}

VMachine QAppWindow::getActiveVm(){
    VMachine vm;
    if( m_selectedVmIndex != -1){
        vm = m_vmList[m_selectedVmIndex];
    }
    return vm;
}

void QAppWindow::doSnapshot(){
    QStringList  snapFullNames;
    SnapManager* snapManager = new SnapManager(this);
    snapFullNames = snapManager->doSnapshot(m_currentVmName, m_mountStorages);
    m_mountStorages = snapFullNames;
    snapManager->deleteLater();
    if (snapFullNames.size() == 0){
        return;
    }

    // *** Обновление дерева снимков состояний виртуальной машины *** //
    // *** Получение индекса модели данных активного состояния VM *** //
    QModelIndex index = m_snapTreeModel->getActiveStateIndex();
    m_snapTreeModel->setSnapImagesFullName(snapFullNames);
    bool ok = m_snapTreeModel->insertRow(index.row(), index);

    // *** Не сворачивать SnapTreeView *** //
    m_snapTreeView->expandAll();

    // *** Выделение и переход к новому узлу *** //
    // QModelIndex newItemIdx = m_snapTreeModel->index(index.row(), 0, index);
    index = m_snapTreeModel->getActiveStateIndex();
    m_snapTreeView->selectionModel()->setCurrentIndex(index,
                                           QItemSelectionModel::ClearAndSelect);
    m_snapTreeView->setFocus();
    m_snapTreeView->scrollTo(index, QAbstractItemView::PositionAtCenter);

    // *** Отключение кнопок Goto и Delete *** //
    m_deleteBtn->setEnabled(false);
    m_gotoBtn->setEnabled(false);
}

void QAppWindow::gotoSnapshot(){
    QStringList snapFullNames;

    // *** Доступ к данным через QItemSelectionModel *** //
    QItemSelectionModel* selectionModel = m_snapTreeView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    // *** Выделение одиночное, в списке максимум один элемент *** //
    QModelIndex index = selectedIndexes[0];

    // *** Получение данных выделенного узла цепочки сохранения состояний *** //
    const ChainNode& node = m_snapTreeModel->getChainNodeByIndex(index);

    SnapManager* snapManager = new SnapManager(this);
    snapFullNames = snapManager->gotoSnapshot(m_currentVmName, node );
    m_mountStorages = snapFullNames;
    snapManager->deleteLater();
    m_snapTreeView->clearFocus();

    // *** Отображение изменений в SnapTreeView *** //
    if (node.imagesType == "work"){
        m_snapTreeModel->setActive(index);
    }
    else {
        qDebug() << index.row();
        m_snapTreeModel->setSnapImagesFullName(snapFullNames);
        bool ok = m_snapTreeModel->insertRowAt(index.row(), index);

        // *** Выделение и переход к новому узлу *** //
        QModelIndex newNodeIndex = m_snapTreeModel->getActiveStateIndex();
        m_snapTreeView->selectionModel()->setCurrentIndex(newNodeIndex,
                                           QItemSelectionModel::ClearAndSelect);
        m_snapTreeView->setFocus();
        m_snapTreeView->scrollTo(newNodeIndex,
                                           QAbstractItemView::PositionAtCenter);
    }

    // *** Отключение кнопок Goto и Delete *** //
    m_deleteBtn->setEnabled(false);
    m_gotoBtn->setEnabled(false);
}

void QAppWindow::deleteSnapshot(){

    // *** Доступ к данным через QItemSelectionModel *** //
    QItemSelectionModel* selectionModel = m_snapTreeView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    // *** Выделение одиночное, в списке максимум один элемент *** //
    QModelIndex index = selectedIndexes[0];

    // *** Получение данных выделенного узла цепочки сохранения состояний *** //
    const ChainNode node = m_snapTreeModel->getChainNodeByIndex(index);

    // *** Проверка попытки удаления активного состояния ВМ *** //
    bool isMount = false;
    for (int i = 0; i < m_mountStorages.size(); ++i){
        if (node.imagesFullNames.contains(m_mountStorages[i])){
            isMount = true;
        }
    }

    if (isMount){
        QMessageBox::information(this, "Deleting snapshot...",
                  "The active state of the virtual machine cannot be deleted.");
        return;
    }

    // *** Запрет удаления корневого узла при наличии нескольких потомков *** //
    if (node.childrenImagesFullNames.size() > 1 && node.parentId == -1 ) {
        QMessageBox::information(this,"Root chain node deletion...",
                "Info: The root snapshot can only be removed with one child.");
        return;
    }

    // *** Удаление снапшота с диска *** //
    SnapManager* snapManager = new SnapManager(this);
    bool doneDelete;
    doneDelete = snapManager->deleteSnapshot(m_currentVmName, node);
    snapManager->deleteLater();

    if (doneDelete == false){
        return;
    }

    // *** Удаление данных из модели данных, привязанной к SnapTreeView *** //
    // 1. Получить текущий индекс (известен ранее "QModelIndex index");
    // 2. Получить индекс родительского узла ("QModelIndex parentIndex");
    // 3. Получить row - номер позиции среди детей одного и того же родителя
    // 3. Вызвать m_snapTreeModel->removeRow(row, parentIndex),
    //    которая лишь обёртка над removeRows(row, 1, parent).
    // 4. Метод removeRows() необходимо реализовать самостоятельно,
    //    где должны быть реализованы:
    //    - уведомление SnapTreeView о том, что ожидается удаление строк(и):
    //        beginRemoveRows(parent, row, row + count - 1);
    //    - удаление узла из модели данных
    //    - уведомление SnapTreeView о завершении операции и изменении данных:
    //        endRemoveRows();

    QModelIndex parentIndex = index.parent();
    bool ok = m_snapTreeModel->removeRow(index.row(), parentIndex);

    // После удаления узла, пересчёта id и parentId для оставшихся,
    // возникает баг, ветки с id бОльшими, чем удалённый узел схлопываются.
    // Правильно предотвратить схлопывание не вышло, используется костыль
    m_snapTreeView->expandAll();
    m_snapTreeView->clearFocus();
    m_snapTreeView->selectionModel()->clear();

    // *** Выдленых элементов SnapTreeView нет, отключение кнопок *** //
    m_deleteBtn->setEnabled(false);
    m_gotoBtn->setEnabled(false);
}

void QAppWindow::startVM(){
    // *** Запуск ВМ, необходимо предупреждение *** //
    QLayoutItem* item = m_vVmLayout->itemAt(m_selectedVmIndex);
    m_vmList[m_selectedVmIndex].state = "Waiting to start, please wait …";
    if (item) {
        VmWidget* widget = static_cast<VmWidget*>(item->widget());
        if (widget) {
            widget->setProperties(m_vmList[m_selectedVmIndex]);
        }
    }

    QProcess process;
    QEventLoop loop;

    bool stepOneDone = false;
    QObject::connect(&process, &QProcess::finished,
                    [&](int, QProcess::ExitStatus){
                        if (!stepOneDone){
                            stepOneDone = true;

                            // Запуск второго этапа старта виртуальной машины
                            process.start("virsh", {"start", m_currentVmName});
                        }
                        else{
                            loop.quit();
                        }
                    });
    // *** Первый этап запуска ВМ *** //
    process.start("virt-manager", {"--connect=qemu:///system",
                                    "--show-domain-console", m_currentVmName});
    loop.exec();
}

void QAppWindow::onTreeItemClicked(const QModelIndex& index){
    QString text = index.data(Qt::DisplayRole).toString();
    qDebug() << "Клик по строке:" << text;

    const ChainNode& node = m_snapTreeModel->getChainNodeByIndex(index);
    m_activeNode = m_snapTreeModel->getChainNodeByIndex(index);
    if (node.id != -1) {
        qDebug() << "        Node Id:" << node.id;
        qDebug() << "      Parent Id:" << node.parentId;
        qDebug() << "    Images Type:" << node.imagesType;
        qDebug() << "ImagesFullNames:";
        for (int i = 0; i < node.imagesFullNames.size(); ++i){
            qDebug() << "                " << node.imagesFullNames[i];
        }
        qDebug() << "   BackFullName:";
        for (int i = 0; i < node.backFullNames.size(); ++i){
            qDebug() << "                " << node.backFullNames[i];
        }
        qDebug() << "   children:" << node.childrenImagesFullNames.size() ;
        for (int i = 0; i < node.childrenImagesFullNames.size(); ++i){
            for (int j = 0; j < node.childrenImagesFullNames[i].size(); ++j ){
                qDebug() << "                "
                            << node.childrenImagesFullNames[i][j];
            }
            qDebug() << "";
        }
    }
}

void QAppWindow::resizeEvent(QResizeEvent* event) {
    // Fix width alternate color "bug". It's only text width
    m_snapTreeView->header()
                       ->setMinimumSectionSize(width() - m_appWindowWidth/2.24);
}

void QAppWindow::takeSnapBtnManage(){
    if (m_mountStorages.size() > 0){
        m_takeSnapBtn->setEnabled(true);
    }
    else {
        m_takeSnapBtn->setEnabled(false);
    }
}

void QAppWindow::gotoAndDelBtnManage(){

    QItemSelectionModel* selectionModel = m_snapTreeView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    if (selectedIndexes.size() > 0){
        QModelIndex index = selectedIndexes[0];
        const ChainNode& node = m_snapTreeModel->getChainNodeByIndex(index);
        // *** "active" нельзя удалить и нельзя в него перейти *** //
        if (node.imagesType != "active"){
            m_deleteBtn->setEnabled(true);
            m_gotoBtn->setEnabled(true);
        }
        else {
            m_deleteBtn->setEnabled(false);
            m_gotoBtn->setEnabled(false);
        }

    }
    else {
        m_deleteBtn->setEnabled(false);
        m_gotoBtn->setEnabled(false);
    }
}

void QAppWindow::startBtnManage(){
    if (m_vmList.size() > 0){
        m_startBtn->setEnabled(true);
    }
    else {
        m_startBtn->setEnabled(false);
    }
}

void QAppWindow::showErrorMessage(const QString& title, const QString& message){
    QMessageBox::critical(this, title,message);
}

void QAppWindow::onVmListReady(const QVector<VMachine> newVmList){

    QVector<VMachine> toAddVmList; // список VM для добавления программу
    QVector<VMachine> toDelVmList; // список для удаления из списка в программе
    QVector<VMachine> toModVmList; // список VM с изменившимся статусом

    toModVmList = getToModVmList(m_vmList, newVmList);
    toAddVmList = getToAddVmList(m_vmList, newVmList);
    toDelVmList = getToDelVmList(m_vmList, newVmList);

    // qDebug() << "\n[II] toModVmList.size()" << toModVmList.size();
    // qDebug() << "[II] toAddVmList.size()" << toAddVmList.size();
    // qDebug() << "[II] toDelVmList.size()" << toDelVmList.size();

    // *** Cбор данных прекращается до окончания их обработки *** //
    if ( toModVmList.size() > 0 || toAddVmList.size() > 0
                                                    || toDelVmList.size() > 0 ){
        emit vmListProcessingStarted();

        // *** Работа со списком VM *** //
        if (toAddVmList.size() > 0){
            addVmToFrame(toAddVmList);
        }

        if (toDelVmList.size() > 0){
            delVmFromFrame(toDelVmList);
        }

        if (toModVmList.size() >0 ){
            modVmIntoFrame(toModVmList);
        }
    }

    emit vmListProcessingCompleted();
}

QVector<VMachine> QAppWindow::getToModVmList(const QVector<VMachine>& appList,
                                               const QVector<VMachine>& inList){
    QVector<VMachine> res;

    // *** Измениться могут только те VM, о которых уже знает программа *** //
    for (int i = 0; i < appList.size(); ++i){
        for (int j = 0; j < inList.size(); ++j){
            if (appList[i].uuid == inList[j].uuid){
                // *** Существующая VM найдена в полученном векторе VM *** //
                // ***   (проверка на изменение статуса или имени VM)  *** //
                if (appList[i].state != inList[j].state
                                          || appList[i].name != inList[j].name){
                    res.push_back(inList[j]);
                }
            }
        }
    }

    return res;
}

QVector<VMachine> QAppWindow::getToAddVmList(const QVector<VMachine>& appList,
                                               const QVector<VMachine>& inList){
    QVector<VMachine> res;
    for (int i = 0; i < inList.size(); ++i){
        size_t entryCounter = 0;
        for (int j = 0; j < appList.size(); ++j){
            if (inList[i].uuid == appList[j].uuid){
                entryCounter++;
            }
        }
        if (entryCounter == 0){
            res.push_back(inList[i]);
        }
    }
    return res;
}

QVector<VMachine> QAppWindow::getToDelVmList(const QVector<VMachine>& appList,
                                               const QVector<VMachine>& inList){
    QVector<VMachine> res;
    for (int i = 0; i < appList.size(); ++i){
        size_t entryCounter = 0;
        for (int j = 0; j < inList.size(); ++j){
            if (appList[i].uuid == inList[j].uuid){
                entryCounter++;
            }
        }
        if (entryCounter == 0){
            res.push_back(appList[i]);
        }
    }
    return res;
}

int QAppWindow::getInsertWidgetIndex(const QVector<VMachine>& appList,
                                                          const VMachine& inVm){
    QStringList vmNameList;

    for (int i = 0; i < appList.size(); ++i){
        vmNameList.push_back(appList[i].name);
    }

    vmNameList.push_back(inVm.name);
    vmNameList.sort(Qt::CaseInsensitive);
    return vmNameList.indexOf(inVm.name);
}

int QAppWindow::getDeleteWidgetIndex(const QVector<VMachine>& appList,
                                                          const VMachine& inVm){
    QStringList vmNameList;

    for (int i = 0; i < appList.size(); ++i){
        vmNameList.push_back(appList[i].name);
    }

    return vmNameList.indexOf(inVm.name);
}

int QAppWindow::getModifyWidgetIndex(const QVector<VMachine>& appList,
                                                          const VMachine& inVm){
    QStringList uuidVmList;

    for (int i = 0; i < appList.size(); ++i){
        uuidVmList.push_back(appList[i].uuid);
    }

    return uuidVmList.indexOf(inVm.uuid);
}

// End appWindow.cpp
