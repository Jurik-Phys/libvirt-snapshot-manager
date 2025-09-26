// Begin appWindow.cpp

#include "appWindow.h"

QAppWindow::QAppWindow(QWidget *parent) : QWidget(parent){

    if (!this->checkExternalVmUtilities()){
        throw std::runtime_error("External utilities failure");
    }

    if (!this->checkLocalHypervisorConnection()){
        throw std::runtime_error("Local hypervisor connection failed");
    }
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

    m_vmDataCollector = new VmDataCollector();
    bool isOkGetVmList = false;
    do {
        m_vmList = m_vmDataCollector->getVmList(&isOkGetVmList);
    } while (!isOkGetVmList);

    QThread* getVmListThread = new QThread();
    m_vmDataCollector->moveToThread(getVmListThread);
    QObject::connect(getVmListThread, &QThread::started,
                         m_vmDataCollector, &VmDataCollector::vmListStartTimer);
    QObject::connect(m_vmDataCollector, &VmDataCollector::vmListReady,
                                              this, &QAppWindow::onVmListReady);
    QObject::connect(this, &QAppWindow::vmListProcessingStarted,
                    m_vmDataCollector, &VmDataCollector::vmListStopTimer,
                                                  Qt::BlockingQueuedConnection);
    QObject::connect(this, &QAppWindow::vmListProcessingCompleted,
                         m_vmDataCollector, &VmDataCollector::vmListStartTimer);
    QObject::connect(this, &QAppWindow::startVmBegin,
                                               this, &QAppWindow::viewOnlyMode);
    QObject::connect(this, &QAppWindow::startVmBegin,
                          m_vmDataCollector, &VmDataCollector::vmListStopTimer);
    QObject::connect(this, &QAppWindow::startVmEnd,
                         m_vmDataCollector, &VmDataCollector::vmListStartTimer);
    QObject::connect(this, &QAppWindow::deleteActiveVm,
                   m_vmDataCollector, &VmDataCollector::vmGeneralInfoStopTimer);

    // vmInfoWidget update
    QObject::connect(m_vmDataCollector, &VmDataCollector::newVmInfoReady,
                                           this, &QAppWindow::onNewVmInfoReady);

    getVmListThread->start();

    m_snapTreeModel = new SnapTreeModel();
    m_snapTreeLoadingModel = new SnapTreeModel();
    ChainNode loadingNode;
    loadingNode.id = 1;
    loadingNode.parentId = -1;
    loadingNode.name = "Loading snapshot chain information. Please wait…";
    m_snapTreeLoadingModel->setSnapData({loadingNode});

    setVmBtnFrame();
    setVmFrame();
    setVmInfoFrame();
    setSnapBtnFrame();
    setSnapFrame();
    setSnapInfoFrame();

    QObject::connect(m_vmInfoWidget, &VmInfoWidget::textChangedBegin,
                   m_vmDataCollector, &VmDataCollector::vmGeneralInfoStopTimer);

    QObject::connect(m_vmInfoWidget, &VmInfoWidget::textChangedEnd,
                m_vmDataCollector,
                    QOverload<>::of(&VmDataCollector::vmGeneralInfoStartTimer));
    // *** Write vmInfo & snapInfo to xml file of VM *** //
    QObject::connect(m_vmInfoWidget, &VmInfoWidget::writeVmTitleRequested,
                             m_vmDataCollector, &VmDataCollector::writeVmTitle);
    QObject::connect(m_vmInfoWidget, &VmInfoWidget::writeVmDescriptionRequested,
                       m_vmDataCollector, &VmDataCollector::writeVmDescription);
    QObject::connect(m_snapInfoWidget, &SnapInfoWidget::writeSnapTitleRequested,
                           m_vmDataCollector, &VmDataCollector::writeSnapTitle);
    QObject::connect(m_snapInfoWidget,
                &SnapInfoWidget::writeSnapDescriptionRequested,
                     m_vmDataCollector, &VmDataCollector::writeSnapDescription);
    // *** Remove empty snapshot information entries *** //
    QObject::connect(m_snapInfoWidget,
                &SnapInfoWidget::removeEmptySnapshotInfoRequested,
                     m_vmDataCollector, &VmDataCollector::rmSnapshotXmlElement);

    addVmToFrame();
}

QAppWindow::~QAppWindow(){
}

void QAppWindow::appExit(){
    this->close();
}

void QAppWindow::closeEvent(QCloseEvent *event) {
   QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm exit", "Do you really want to exit?",
                                            QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        event->accept();
    }
    else{
        event->ignore();
    }
}

void QAppWindow::setVmBtnFrame(){
    QFrame* vmBtnFrame = new QFrame(this);
    vmBtnFrame->setFrameShape(QFrame::StyledPanel);
    vmBtnFrame->setFrameShadow(QFrame::Plain);
    vmBtnFrame->setFixedHeight(m_headFrameHeight);
    vmBtnFrame->setFixedWidth(m_appWindowWidth/2.5);

    QHBoxLayout* vmBtnFrameHLayout = new QHBoxLayout(vmBtnFrame);

    QIcon startBtnIcon(":/btn-vm-start.svg");
    m_startBtn = new QToolButton(vmBtnFrame);
    m_startBtn->setText("Start");
    m_startBtn->setFixedHeight(m_btnHeight);
    m_startBtn->setFixedWidth(m_btnWidth);
    m_startBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_startBtn->setAutoRaise(true);
    m_startBtn->setIcon(startBtnIcon);
    m_startBtn->setIconSize(QSize(48,48));
    QObject::connect(m_startBtn, &QToolButton::clicked,
                                                    this, &QAppWindow::startVM);
    m_startBtn->setEnabled(false);

    QIcon pauseBtnIcon(":/btn-vm-pause.svg");
    m_pauseBtn = new QToolButton(vmBtnFrame);
    m_pauseBtn->setText("Pause");
    m_pauseBtn->setFixedHeight(m_btnHeight);
    m_pauseBtn->setFixedWidth(m_btnWidth);
    m_pauseBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pauseBtn->setEnabled(false);
    m_pauseBtn->setAutoRaise(true);
    m_pauseBtn->setIcon(pauseBtnIcon);
    m_pauseBtn->setIconSize(QSize(48,48));
    QObject::connect(m_pauseBtn, &QToolButton::clicked,
                                              this, &QAppWindow::togglePauseVM);

    QIcon stopBtnIcon(":/btn-vm-stop.svg");
    m_stopBtn = new QToolButton(vmBtnFrame);
    m_stopBtn->setText("Stop");
    m_stopBtn->setFixedHeight(m_btnHeight);
    m_stopBtn->setFixedWidth(m_btnWidth);
    m_stopBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_stopBtn->setPopupMode(QToolButton::InstantPopup);
    m_stopBtn->setEnabled(false);
    m_stopBtn->setAutoRaise(true);
    m_stopBtn->setIcon(stopBtnIcon);
    m_stopBtn->setIconSize(QSize(48,48));

    // *** Два варианта меню для кнопки stop *** //
    m_fullStopBtnMenu = new QMenu(m_stopBtn);
    QAction* actReboot = m_fullStopBtnMenu->addAction("Reboot");
    QAction* actForceReboot = m_fullStopBtnMenu->addAction("Force reboot");
    QAction* actShutdown = m_fullStopBtnMenu->addAction("Shutdown");
    QAction* actForceShutdown = m_fullStopBtnMenu->addAction("Force shutdown");

    m_onlyForceStopBtnMenu = new QMenu(m_stopBtn);
    m_onlyForceStopBtnMenu->addAction(actForceReboot);
    m_onlyForceStopBtnMenu->addAction(actForceShutdown);

    m_defaulActionList.push_back(actShutdown);
    m_defaulActionList.push_back(actForceShutdown);

    QObject::connect(actReboot, &QAction::triggered,
                                               this, &QAppWindow::menuRebootVM);
    QObject::connect(actShutdown, &QAction::triggered,
                                             this, &QAppWindow::menuShutDownVM);
    QObject::connect(actForceReboot, &QAction::triggered,
                                          this, &QAppWindow::menuForceRebootVM);
    QObject::connect(actForceShutdown, &QAction::triggered,
                                        this, &QAppWindow::menuForceShutdownVM);

    QFrame* vLine = new QFrame(vmBtnFrame);
    vLine->setFrameShape(QFrame::VLine);
    vLine->setFrameShadow(QFrame::Plain);
    vLine->setFixedWidth(3);
    vLine->setFixedHeight(m_btnHeight + 0.05*m_btnHeight);
    QColor borderColor = vmBtnFrame->palette().color(QPalette::Mid);
    vLine->setStyleSheet(QString("color: %1;").arg(borderColor.name()));

    QIcon openBtnIcon(":/btn-vm-open.png");
    m_openBtn = new QToolButton(vmBtnFrame);
    m_openBtn->setText("Open");
    m_openBtn->setFixedHeight(m_btnHeight);
    m_openBtn->setFixedWidth(m_btnWidth);
    m_openBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_openBtn->setAutoRaise(true);
    m_openBtn->setIcon(openBtnIcon);
    m_openBtn->setIconSize(QSize(48,48));
    QObject::connect(m_openBtn, &QToolButton::clicked, this,
                                                           &QAppWindow::openVM);

    vmBtnFrameHLayout->addWidget(m_startBtn);
    vmBtnFrameHLayout->addWidget(m_pauseBtn);
    vmBtnFrameHLayout->addWidget(m_stopBtn);
    vmBtnFrameHLayout->addWidget(vLine);
    vmBtnFrameHLayout->addWidget(m_openBtn);

    m_vLColumnLayout->addWidget(vmBtnFrame);

    QObject::connect(this, &QAppWindow::selectVmChanged,
                                             this, &QAppWindow::btnManageStart);
    QObject::connect(this, &QAppWindow::selectVmChanged,
                                             this, &QAppWindow::btnManageStop);
    QObject::connect(this, &QAppWindow::selectVmChanged,
                                             this, &QAppWindow::btnManagePause);
    QObject::connect(this, &QAppWindow::selectVmChanged,
                                             this, &QAppWindow::btnManageGoDel);
    QObject::connect(this, &QAppWindow::selectVmChanged,
                                             this, &QAppWindow::btnManageTake);
    QObject::connect(this, &QAppWindow::selectVmChanged,
                                         this, &QAppWindow::snapTreeViewManage);
    QObject::connect(this, &QAppWindow::selectVmChanged,
                                         this, &QAppWindow::menuStopBtnSelect);
    QObject::connect(this, &QAppWindow::selectVmChanged,
                                         this, &QAppWindow::vmInfoWidgetManage);
    QObject::connect(this, &QAppWindow::selectVmChanged,
                                       this, &QAppWindow::snapInfoWidgetManage);
}

void QAppWindow::setVmFrame(){
    QScrollArea* vmFrame = new QScrollArea(this);
    vmFrame->setWidgetResizable(true);

    QWidget* container = new QWidget(vmFrame);
    container->setLayout(m_vVmLayout);
    vmFrame->setWidget(container);

    vmFrame->viewport()->setStyleSheet("background-color: white;");
    vmFrame->setFixedWidth(m_appWindowWidth/2.5);

    m_vLColumnLayout->addWidget(vmFrame);
}

void QAppWindow::setVmInfoFrame(){
    m_vmInfoWidget = new VmInfoWidget(this);
    m_vmInfoWidget->setFrameShape(QFrame::StyledPanel);
    m_vmInfoWidget->setFrameShadow(QFrame::Plain);
    m_vmInfoWidget->setFixedHeight(m_infoFrameHeight);
    m_vmInfoWidget->setFixedWidth(m_appWindowWidth/2.5);

    // *** Правильная установка белого фона *** //
    m_vmInfoWidget->setAutoFillBackground(true);
    QPalette pal = m_vmInfoWidget->palette();
    pal.setColor(QPalette::Window, Qt::white);
    m_vmInfoWidget->setPalette(pal);

    m_vLColumnLayout->addWidget(m_vmInfoWidget);
}

void QAppWindow::setSnapBtnFrame(){
    QFrame* snapBtnFrame = new QFrame(this);
    snapBtnFrame->setFrameShape(QFrame::StyledPanel);
    snapBtnFrame->setFrameShadow(QFrame::Plain);
    snapBtnFrame->setFixedHeight(m_headFrameHeight);

    QHBoxLayout* snapBtnFrameHLayout = new QHBoxLayout(snapBtnFrame);

    QIcon gotoBtnIcon(":/btn-goto-snapshot.png");
    m_gotoBtn = new QToolButton(snapBtnFrame);
    m_gotoBtn->setText("Go to");
    m_gotoBtn->setFixedHeight(m_btnHeight);
    m_gotoBtn->setFixedWidth(m_btnWidth);
    m_gotoBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_gotoBtn->setEnabled(false);
    m_gotoBtn->setAutoRaise(true);
    m_gotoBtn->setIcon(gotoBtnIcon);
    m_gotoBtn->setIconSize(QSize(48,48));
    QObject::connect(m_gotoBtn, &QToolButton::clicked, this,
                                                     &QAppWindow::gotoSnapshot);

    QIcon deleteBtnIcon(":/btn-delete-snapshot.svg");
    m_deleteBtn = new QToolButton(snapBtnFrame);
    m_deleteBtn->setText("Delete");
    m_deleteBtn->setFixedHeight(m_btnHeight);
    m_deleteBtn->setFixedWidth(m_btnWidth);
    m_deleteBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_deleteBtn->setEnabled(false);
    m_deleteBtn->setAutoRaise(true);
    m_deleteBtn->setIcon(deleteBtnIcon);
    m_deleteBtn->setIconSize(QSize(48,48));
    QObject::connect(m_deleteBtn, &QToolButton::clicked, this,
                                                   &QAppWindow::deleteSnapshot);
    QFrame* vLine = new QFrame(snapBtnFrame);
    vLine->setFrameShape(QFrame::VLine);
    vLine->setFrameShadow(QFrame::Plain);
    vLine->setFixedWidth(3);
    vLine->setFixedHeight(m_btnHeight + 0.05*m_btnHeight);
    QColor borderColor = snapBtnFrame->palette().color(QPalette::Mid);
    vLine->setStyleSheet(QString("color: %1;").arg(borderColor.name()));

    QIcon takeSnapBtnIcon(":/btn-take-snapshot.svg");
    m_takeSnapBtn = new QToolButton(snapBtnFrame);
    m_takeSnapBtn->setText("Take");
    m_takeSnapBtn->setFixedHeight(m_btnHeight);
    m_takeSnapBtn->setFixedWidth(m_btnWidth);
    m_takeSnapBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_takeSnapBtn->setEnabled(false);
    m_takeSnapBtn->setAutoRaise(true);
    m_takeSnapBtn->setIcon(takeSnapBtnIcon);
    m_takeSnapBtn->setIconSize(QSize(48,48));
    QObject::connect(m_takeSnapBtn, &QToolButton::clicked, this,
                                                       &QAppWindow::doSnapshot);

    QIcon exitBtnIcon(":/btn-app-exit.png");
    QToolButton* exitBtn = new QToolButton(snapBtnFrame);
    exitBtn->setText("Exit");
    exitBtn->setFixedHeight(m_btnHeight);
    exitBtn->setFixedWidth(m_btnWidth);
    exitBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    exitBtn->setAutoRaise(true);
    exitBtn->setIcon(exitBtnIcon);
    exitBtn->setIconSize(QSize(48,48));
    QObject::connect(exitBtn, &QToolButton::clicked, this,&QAppWindow::appExit);

    snapBtnFrameHLayout->addWidget(m_gotoBtn);
    snapBtnFrameHLayout->addWidget(m_deleteBtn);
    snapBtnFrameHLayout->addWidget(vLine);
    snapBtnFrameHLayout->addWidget(m_takeSnapBtn);
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
                                              this, &QAppWindow::btnManageGoDel);
    QObject::connect(m_snapTreeView, &SnapTreeView::doSnapshot,
                                              this, &QAppWindow::doSnapshot);
    QObject::connect(m_snapTreeView, &SnapTreeView::deleteSnapshot,
                                             this, &QAppWindow::deleteSnapshot);
    QObject::connect(m_snapTreeView, &SnapTreeView::gotoSnapshot,
                                             this, &QAppWindow::gotoSnapshot);
    QObject::connect(m_snapTreeView, &SnapTreeView::startVM,
                                                    this, &QAppWindow::startVM);

    // *** Отключение всплывающего меню  *** //
    m_snapTreeView->setContextMenuPolicy(Qt::NoContextMenu);
}

void QAppWindow::setSnapInfoFrame(){
    m_snapInfoWidget = new SnapInfoWidget(this);
    m_snapInfoWidget->setFrameShape(QFrame::StyledPanel);
    m_snapInfoWidget->setFrameShadow(QFrame::Plain);
    m_snapInfoWidget->setFixedHeight(m_infoFrameHeight);

    // *** Правильная установка белого фона *** //
    m_snapInfoWidget->setAutoFillBackground(true);
    QPalette pal = m_snapInfoWidget->palette();
    pal.setColor(QPalette::Window, Qt::white);
    m_snapInfoWidget->setPalette(pal);

    m_vRColumnLayout->addWidget(m_snapInfoWidget);
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
        vmWidget->setFixedHeight(m_vmWidgetHeight);
        m_vVmLayout->addWidget(vmWidget);
        m_vVmLayout->setSpacing(3);
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
                                                     &QAppWindow::viewOnlyMode);
        QObject::connect(vmWidget, &VmWidget::clicked, this,
                                                &QAppWindow::menuStopBtnSelect);
        QObject::connect(vmWidget, &VmWidget::clicked, m_snapInfoWidget,
                                                    &SnapInfoWidget::clearData);
    }
}

void QAppWindow::addVmToFrame(const QVector<VMachine>& toAddvmList){
    for (int i = 0; i < toAddvmList.size(); ++i){
        // *** Индекс вставки VM *** //
        int indexWidget = getInsertWidgetIndex(m_vmList, toAddvmList[i]);
        // *** Новую VM надо добавить не только на форму, но и в список *** //
        m_vmList.insert(indexWidget, toAddvmList[i]);

        VmWidget* vmWidget = new VmWidget(toAddvmList[i], this);
        vmWidget->setFixedHeight(m_vmWidgetHeight);
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
                                                     &QAppWindow::viewOnlyMode);
        QObject::connect(vmWidget, &VmWidget::clicked, this,
                                                &QAppWindow::menuStopBtnSelect);
        QObject::connect(vmWidget, &VmWidget::clicked, m_snapInfoWidget,
                                                    &SnapInfoWidget::clearData);
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

        // *** Изменения произошли в выделенной сейчас ВМ ? *** //
        bool isSelectVM = false;
        if ( modIdx == m_selectedVmIndex ){
            isSelectVM = true;
        }

        QLayoutItem* item = m_vVmLayout->itemAt(modIdx);
        if (item) {
            VmWidget* widget = static_cast<VmWidget*>(item->widget());
            if (widget) {
                widget->setProperties(toModVmList[i]);

                // *** Изменение свойств в списке виртуальных машин *** //
                if (m_vmList[modIdx].name  != toModVmList[i].name){
                    // *** Изменяем имя виртуальной машины в списке машин *** //
                    m_vmList[modIdx].name  = toModVmList[i].name;
                    // *** Перезапись статуса ВМ на случай его изменения *** //
                    m_vmList[modIdx].state = toModVmList[i].state;

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

                    // *** Фиксация изменения индекса выделенной ВМ *** //
                    if (isSelectVM){
                        m_selectedVmIndex = newModIdx;
                    }
                }
                else {
                    // Обновление статуса в списке виртуальных машин;
                    m_vmList[modIdx].state = toModVmList[i].state;
                }

                // *** Сигнал об изменениях в выделенной ВМ *** //
                // *** Управление кнопками (start|pause|stop|take) *** //
                if (isSelectVM){
                    emit selectVmChanged();
                }
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
                m_currentVmUUID = result.uuid;

                m_vmInfoWidget->setData(result);
                // qDebug() << "[II] Данные получены и выведены (finished)";

                // *** On/Off buttons *** //
                btnManageGoDel();
                btnManageTake();
                btnManageStart();
                btnManagePause();
                btnManageStop();
                snapTreeViewManage();
                vmInfoWidgetManage();
                snapInfoWidgetManage();

                thread->quit();
                thread->wait();

                vmDataCollector->deleteLater();
                thread->deleteLater();

                // *** Снятие флага продолжения загрузки *** //
                vmWidget->setLoadingFlag(false);
        });

    // После получения данных от локального vmDataCollector,
    // запуск таймера в глобальном m_vmDataCollector на обновление информации ВМ
    QObject::connect(vmDataCollector, &VmDataCollector::finished,
    m_vmDataCollector,
    QOverload<const VMachine&>::of(&VmDataCollector::vmGeneralInfoStartTimer));
}

VMachine QAppWindow::getActiveVm(){
    VMachine vm;
    if( m_selectedVmIndex != -1){
        vm = m_vmList[m_selectedVmIndex];
    }
    return vm;
}

bool QAppWindow::checkExternalVmUtilities(){
    QStringList utilsList = {"virsh", "qemu-img", "virt-manager"};
    bool res = true;

    for (int i = 0; i < utilsList.size(); ++i){
        if (!checkVmUtilityAvailable(utilsList[i])){
            res = false;
            break;
        }
        else {
            if (!checkVmUtilityExecutable(utilsList[i])){
                res = false;
                break;
            }
        }
    }

    return res;
}

bool QAppWindow::checkVmUtilityAvailable(const QString& utilityName){
    bool res = false;
    QString path = QStandardPaths::findExecutable(utilityName);
    res = !path.isEmpty();

    if (!res){
        QString title = "External utilities failure";
        QString message = "Utility \"" + utilityName
                                          + "\" not found. Please install it.";
        QMessageBox::critical(nullptr, title, message);
    }

    return res;
}

bool QAppWindow::checkVmUtilityExecutable(const QString& utilityName){
    bool res = false;

    QEventLoop loop;
    QProcess process;

    QObject::connect(&process, &QProcess::finished, [&](){

            // ************************** Теория **************************** //
            // Для проверки запуска утилиты, производится попытка её запуска  //
            // с параметром "--help" для вывода справочной информации.        //
            // Успешный анализ справочной информации, общепринято содержащей  //
            // описание параметров "--help" или "-h" считается успешным       //
            // запуском исследуемой утилиты.                                  //
            // ************************************************************** //

            QString output = process.readAllStandardOutput();
            static const QRegularExpression re(R"(-h|--help)");
            QRegularExpressionMatchIterator it = re.globalMatch(output);

            int count = 0;
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                count++;
            }

            if (count > 0){
                res = true;
            }
            loop.quit();
        });

    process.start(utilityName, {"--help"});
    loop.exec();

    return res;
}

bool QAppWindow::checkLocalHypervisorConnection(){
    // ******************************* Теория ******************************* //
    // > Проблема. Не существует способа проверить и возможность подключения  //
    //   к гипервизору и не взывать диалог авторизации Polkit'а,              //
    //   если он испльзуется для контроля доступа к гипервизору.              //
    //   Даже вызов функции virConnectOpen() библиотеки libvirt из данной     //
    //   программы вызывает диалог авторизации (занавес).                     //
    //                                                                        //
    // > Решение. п.1. Определить через парсинг файла настроек демона libvirt //
    //   "/etc/libvirt/libvirtd.conf" значение параметра "auth_unix_rw".      //
    //   Если параметра не существует или он закомментирован, то по умолчанию //
    //   polkit, если параметр равен "polkit", то тоже используется Polkit.   //
    //   п.2. Проверить права управления гипервизором текущего пользователя   //
    //   через анализ Exit code команды                                       //
    //            "pkcheck --action-id org.libvirt.unix.manage --process $$"  //
    //   При наличии прав вывод у команды будет пустой и exit code "0",       //
    //   при ошибках доступа или отсутствия зарегистрированного правила на    //
    //   стандартном выводе появитя соответствущее собщение и код выхода      //
    //   будет отличен от нуля.                                               //
    // ********************************************************************** //

    bool res = false;

    if (isLibvirtPolkitEnabled()){
        QString util = "pkcheck";
        if (checkVmUtilityAvailable(util) && checkVmUtilityExecutable(util)){
            QProcess process;
            process.start("pkcheck", {"--action-id",
                                            "org.libvirt.unix.manage",
                                                                "--process",
                          QString::number(QCoreApplication::applicationPid())});
            process.waitForFinished();

            QString output = process.readAllStandardOutput();
            int code = process.exitCode();

            if (code == 0) {
                res = true;
            }
        }
    }
    else {
        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("LANG", "C");
        process.setProcessEnvironment(env);
        process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                              "list", "--all"});
        process.waitForFinished();
        QString output = process.readAllStandardOutput();
        if (!output.contains("failed to connect to the hypervisor")){
            res = true;
        }
    }

    if (!res){
        QString title = "Hypervisor access denied";
        QString message = "Hypervisor user access check failed.\n"
            "Use the following command to manually check access:\n"
            "$ virsh --connect=" + m_libVirtConnectURI + " list --all";
        QMessageBox::critical(nullptr, title, message);
    }

    return res;
}

bool QAppWindow::isLibvirtPolkitEnabled(){
    bool res = true;
    QString libvirtConfFileName = "/etc/libvirt/libvirtd.conf";
    QFile libvirtConfFile(libvirtConfFileName);

    if (libvirtConfFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        while (!libvirtConfFile.atEnd()){
            QByteArray line = libvirtConfFile.readLine();
            QString trStr = line.trimmed();
            if (trStr.size() > 0){
                if (trStr.at(0) != "#" && trStr.contains("auth_unix_rw")
                                                  && !trStr.contains("polkit")){
                    res = false;
                }
            }
        }
    }
    else {
        throw std::runtime_error(("Error read " +
                                     libvirtConfFileName).toLocal8Bit().data());
    }

    return res;
}

void QAppWindow::doSnapshot(){
    QStringList  snapFullNames;
    SnapManager* snapManager = new SnapManager(this);
    snapFullNames = snapManager->doSnapshot(m_currentVmName, m_mountStorages);
    // *** Снапшотов не получилось, пропуск дальнейших действий *** //
    if (snapFullNames.size() == 0){
        return;
    }
    m_mountStorages = snapFullNames;
    snapManager->deleteLater();

    // *** Обновление дерева снимков состояний виртуальной машины *** //
    // *** Получение индекса модели данных активного состояния VM *** //
    QModelIndex index = m_snapTreeModel->getActiveStateIndex();
    m_snapTreeModel->setSnapImagesFullName(snapFullNames);
    bool ok = m_snapTreeModel->insertRow(index.row(), index);

    // *** Не сворачивать SnapTreeView *** //
    m_snapTreeView->expandAll();

    // *** Выделение и переход к новому узлу *** //
    index = m_snapTreeModel->getActiveStateIndex();
    m_snapTreeView->selectionModel()->setCurrentIndex(index,
                                           QItemSelectionModel::ClearAndSelect);
    m_snapTreeView->setFocus();
    m_snapTreeView->scrollTo(index, QAbstractItemView::PositionAtCenter);

    // *** Отключение кнопок Goto и Delete *** //
    m_deleteBtn->setEnabled(false);
    m_gotoBtn->setEnabled(false);

    // *** Обновление информации о примонтированных дисках *** //
    m_vmInfoWidget->setStorageList(m_mountStorages);

    // *** Установка параметров виджета вывода информации *** //
    ChainNode updNode = m_snapTreeModel->getChainNodeByIndex(index);
    m_vmDataCollector->getSnapshotXmlInfo(updNode);
    m_snapInfoWidget->setData(updNode);
}

void QAppWindow::gotoSnapshot(){
    QStringList snapFullNames;

    QModelIndex currentModelIndex;

    // *** Доступ к данным через QItemSelectionModel *** //
    QItemSelectionModel* selectionModel = m_snapTreeView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    // *** Выделение одиночное, в списке максимум один элемент *** //
    QModelIndex index = selectedIndexes[0];
    currentModelIndex = index;

    // *** Получение данных выделенного узла цепочки сохранения состояний *** //
    const ChainNode& node = m_snapTreeModel->getChainNodeByIndex(index);

    SnapManager* snapManager = new SnapManager(this);
    snapFullNames = snapManager->gotoSnapshot(m_currentVmName, node );

    // *** Пропуск перехода к снапшоту, если *work* создать не получилось *** //
    if (snapFullNames.size() == 0){
        return;
    }

    m_mountStorages = snapFullNames;
    snapManager->deleteLater();
    m_snapTreeView->clearFocus();

    // *** Отображение изменений в SnapTreeView *** //
    if (node.imagesType == "work"){
        m_snapTreeModel->setActive(index);
    }
    else {
        m_snapTreeModel->setSnapImagesFullName(snapFullNames);
        bool ok = m_snapTreeModel->insertRowAt(index.row(), index);

        // *** Выделение и переход к новому узлу *** //
        QModelIndex newNodeIndex = m_snapTreeModel->getActiveStateIndex();
        m_snapTreeView->selectionModel()->setCurrentIndex(newNodeIndex,
                                           QItemSelectionModel::ClearAndSelect);
        m_snapTreeView->setFocus();
        m_snapTreeView->scrollTo(newNodeIndex,
                                           QAbstractItemView::PositionAtCenter);
        currentModelIndex = newNodeIndex;
    }

    // *** Отключение кнопок Goto и Delete *** //
    m_deleteBtn->setEnabled(false);
    m_gotoBtn->setEnabled(false);

    // *** Обновление информации о примонтированных дисках *** //
    m_vmInfoWidget->setStorageList(m_mountStorages);

    // *** Установка параметров виджета вывода информации *** //
    ChainNode updNode = m_snapTreeModel->getChainNodeByIndex(currentModelIndex);
    m_vmDataCollector->getSnapshotXmlInfo(updNode);
    m_snapInfoWidget->setData(updNode);
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
        QMessageBox::information(this, "Deleting snapshot",
                  "The active state of the virtual machine cannot be deleted.");
        return;
    }

    // *** Запрет удаления корневого узла при наличии нескольких потомков *** //
    if (node.childrenImagesFullNames.size() > 1 && node.parentId == -1 ) {
        QMessageBox::information(this,"Root chain node deletion",
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

    // *** Выделены элементов updSnapTree нет, сбро информации о снапшоте *** //
    m_snapInfoWidget->clearData();

    // *** Удалить запись о снапшоте из xml документа ВМ *** //
    m_vmDataCollector->rmSnapshotXmlElement({m_currentVmUUID, node.uuid});
}

void QAppWindow::startVM(){

    emit startVmBegin();

    // *** Запуск ВМ, необходимо предупреждение *** //
    QLayoutItem* item = m_vVmLayout->itemAt(m_selectedVmIndex);
    m_vmList[m_selectedVmIndex].state = "Preparing to start, please wait…";
    if (item) {
        VmWidget* widget = static_cast<VmWidget*>(item->widget());
        if (widget) {
            widget->setProperties(m_vmList[m_selectedVmIndex]);
        }
    }

    QProcess process;
    QEventLoop loop;

    QObject::connect(&process, &QProcess::finished,
                    [&](int, QProcess::ExitStatus){
                        emit startVmEnd();
                        loop.quit();
                    });

    process.start("virsh", {"--connect=" + m_libVirtConnectURI,"start",
                                                              m_currentVmName});
    loop.exec();
}

void QAppWindow::togglePauseVM(){

    // *** do resume VM *** //
    if (m_vmList[m_selectedVmIndex].state == "paused"){
        QProcess process;
        QEventLoop loop;
        QObject::connect(&process, &QProcess::finished,
                    [&](int, QProcess::ExitStatus){
                        loop.quit();
                    });
        process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                    "resume", m_currentVmName});
        loop.exec();
    }

    // *** do pause VM *** //
    if (m_vmList[m_selectedVmIndex].state == "running"){
        QProcess process;
        QEventLoop loop;
        QObject::connect(&process, &QProcess::finished,
                    [&](int, QProcess::ExitStatus){
                        loop.quit();
                    });
        process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                   "suspend", m_currentVmName});
        loop.exec();
    }
}

void QAppWindow::openVM(){
    QProcess process;
    process.startDetached("virt-manager", {"--connect=" + m_libVirtConnectURI,
                                     "--show-domain-console", m_currentVmName});
}

void QAppWindow::onTreeItemClicked(const QModelIndex& index){
    QString text = index.data(Qt::DisplayRole).toString();

    ChainNode node = m_snapTreeModel->getChainNodeByIndex(index);
      m_activeNode = m_snapTreeModel->getChainNodeByIndex(index);
    if (node.id != -1) {
        // *** Установка параметров виджета вывода информации *** //
        m_vmDataCollector->getSnapshotXmlInfo(node);
        m_snapInfoWidget->setData(m_vmList[m_selectedVmIndex]);
        m_snapInfoWidget->setData(node);
    }
}

void QAppWindow::resizeEvent(QResizeEvent* event) {
    // Fix width alternate color "bug". It's only text width
    m_snapTreeView->header()
                       ->setMinimumSectionSize(width() - m_appWindowWidth/2.24);
}

void QAppWindow::btnManageTake(){
    if (m_mountStorages.size() > 0
                            && m_vmList[m_selectedVmIndex].state == "shut off"
                                            && m_snapTreeModel->rowCount() > 0){
        m_takeSnapBtn->setEnabled(true);
    }
    else {
        m_takeSnapBtn->setEnabled(false);
    }
}

void QAppWindow::btnManageGoDel(){

    QItemSelectionModel* selectionModel = m_snapTreeView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    if (selectedIndexes.size() > 0){
        QModelIndex index = selectedIndexes[0];
        const ChainNode& node = m_snapTreeModel->getChainNodeByIndex(index);
        // *** "active" нельзя удалить и нельзя в него перейти *** //
        if (node.imagesType != "active"
                           && m_vmList[m_selectedVmIndex].state == "shut off" ){
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

void QAppWindow::btnManageStart(){
    if (m_vmList[m_selectedVmIndex].state == "shut off"){
        m_startBtn->setEnabled(true);
    }
    else {
        m_startBtn->setEnabled(false);
    }
}

void QAppWindow::btnManagePause(){
    if (m_vmList[m_selectedVmIndex].state == "paused"
                            || m_vmList[m_selectedVmIndex].state == "running" ){
        m_pauseBtn->setEnabled(true);
        if (m_vmList[m_selectedVmIndex].state == "paused"){
            m_pauseBtn->setText("Resume");
        }
        else {
            m_pauseBtn->setText("Pause");
        }
    }
    else {
        m_pauseBtn->setEnabled(false);
    }
}

void QAppWindow::btnManageStop(){
    if (m_vmList[m_selectedVmIndex].state == "shut off"){
        // *** Скрыть menu-indicator *** //
        m_stopBtn->setStyleSheet("QToolButton::menu-indicator { image:none;}");
        m_stopBtn->setEnabled(false);
    }
    else {
        // *** Сброс скрытия menu-indicator *** //
        m_stopBtn->setStyleSheet("");
        m_stopBtn->setEnabled(true);
    }
}

void QAppWindow::snapTreeViewManage(){
    if (m_vmList[m_selectedVmIndex].state == "shut off"){
        // *** Режим работы с деревом снапшотов *** //
        m_snapTreeView->setContextMenuPolicy(Qt::DefaultContextMenu);
    }
    else {
        // *** Режим read-only *** //
        m_snapTreeView->setContextMenuPolicy(Qt::NoContextMenu);
    }
}

void QAppWindow::vmInfoWidgetManage(){

    if (m_vmList[m_selectedVmIndex].state == "shut off"){
        m_vmInfoWidget->setReadOnly(false);
    }
    else {
        m_vmInfoWidget->setReadOnly(true);
    }
}

void QAppWindow::snapInfoWidgetManage(){

    bool isSelectSnapshot = m_snapTreeView->isSelectItem();

    // *** Если снапшот не выделен, то режим только чтение *** //
    if (!isSelectSnapshot){
        m_snapInfoWidget->setReadOnly(true);
    }
    else {
        // *** Если выделен снапшот, то возможны варианты *** //
        if (m_vmList[m_selectedVmIndex].state == "shut off"){
            m_snapInfoWidget->setReadOnly(false);
        }
        else {
            m_snapInfoWidget->setReadOnly(true);
        }
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

    // *** Cбор данных прекращается до окончания их обработки *** //
    if ( toModVmList.size() > 0 || toAddVmList.size() > 0
                                                    || toDelVmList.size() > 0 ){
        // Блокирующий вызов Qt::BlockingQueuedConnection
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

void QAppWindow::menuRebootVM(){
    QProcess process;
    QEventLoop loop;
    QObject::connect(&process, &QProcess::finished,
                [&](int, QProcess::ExitStatus){
                    loop.quit();
                });
    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                    "reboot", m_currentVmName});
    loop.exec();
};

void QAppWindow::menuShutDownVM(){
    QProcess process;
    QEventLoop loop;
    QObject::connect(&process, &QProcess::finished,
                [&](int, QProcess::ExitStatus){
                    loop.quit();
                });
    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                  "shutdown", m_currentVmName});
    loop.exec();
}

void QAppWindow::menuForceRebootVM(){
    QProcess process;
    QEventLoop loop;
    QObject::connect(&process, &QProcess::finished,
                [&](int, QProcess::ExitStatus){
                    loop.quit();
                });
    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                     "reset", m_currentVmName});
    loop.exec();
}

void QAppWindow::menuForceShutdownVM(){
    QProcess process;
    QEventLoop loop;
    QObject::connect(&process, &QProcess::finished,
                [&](int, QProcess::ExitStatus){
                    btnManageGoDel();
                    btnManageStop();
                    loop.quit();
                });
    process.start("virsh", {"--connect=" + m_libVirtConnectURI,
                                                   "destroy", m_currentVmName});
    loop.exec();
}

void QAppWindow::menuStopBtnSelect(){
    if (m_vmList[m_selectedVmIndex].state == "paused") {
        m_stopBtn->setMenu(m_onlyForceStopBtnMenu);
        QString text = m_stopBtn->text();
        m_stopBtn->setDefaultAction(m_defaulActionList[1]);
        m_stopBtn->setText(text);
    } else {
        if (m_vmList[m_selectedVmIndex].state == "running") {
            m_stopBtn->setMenu(m_fullStopBtnMenu);
            QString text = m_stopBtn->text();
            m_stopBtn->setDefaultAction(m_defaulActionList[0]);
            m_stopBtn->setText(text);
        }
    }
    QIcon stopBtnIcon(":/btn-vm-stop.svg");
    m_stopBtn->setIcon(stopBtnIcon);
    m_stopBtn->setIconSize(QSize(48,48));
}

void QAppWindow::viewOnlyMode(){
    // *** Отклчюение управлением снапшотами *** //
    m_takeSnapBtn->setEnabled(false);
    m_gotoBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
    m_snapTreeView->setContextMenuPolicy(Qt::NoContextMenu);
    m_snapInfoWidget->setReadOnly(true);
    m_vmInfoWidget->setReadOnly(true);
}

void QAppWindow::onNewVmInfoReady(const VMachine& vmNew){

    // *** Если от vmDataCollector прилетела пустая VM, *** //
    //     то значит текущая VM удалена через virsh, virt-manager и т.д.
    if (vmNew.name == "" && vmNew.uuid == ""){
        // *** Остановка сбора информации т.к., после удаления VM
        //     из списка, в списке нет активных VM
        //     (сигнал т.к., таймер в другом потоке)
        emit deleteActiveVm();
        m_vmInfoWidget->clearData();
        m_snapInfoWidget->clearData();
        QVector<ChainNode> fakeChain;
        m_snapTreeModel->setSnapData(fakeChain);
        m_snapTreeView->viewport()->update();
        return;
    }

    VMachine vmOld = m_vmInfoWidget->getData();
    // Требование равенства uuid обусловлено возможной несинхроностью
    // при переключении между виртуальными машинами.
    // Наличие uuid не позволит испортить записи соседних машин
    if (vmNew.name != vmOld.name && vmNew.uuid == vmOld.uuid){
        m_vmInfoWidget->setName(vmNew.name);
    }

    if (vmNew.title != vmOld.title && vmNew.uuid == vmOld.uuid){
        m_vmInfoWidget->setTitle(vmNew.title);
    }

    if (vmNew.description != vmOld.description && vmNew.uuid == vmOld.uuid){
        m_vmInfoWidget->setDescription(vmNew.description);
    }

    if (vmNew.cpu != vmOld.cpu && vmNew.uuid == vmOld.uuid){
        m_vmInfoWidget->setCpu(vmNew.cpu);
    }

    if (vmNew.ram != vmOld.ram && vmNew.uuid == vmOld.uuid){
        m_vmInfoWidget->setRam(vmNew.ram);
    }

    if (vmNew.osId != vmOld.osId && vmNew.uuid == vmOld.uuid){
        m_vmInfoWidget->setOsId(vmNew.osId);
    }
}

// End appWindow.cpp
