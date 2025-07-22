// Begin appWindow.cpp

#include "appWindow.h"

QAppWindow::QAppWindow(QWidget *parent) : QWidget(parent){
    this->resize(m_appWindowWidth, m_appWindowHeight);
    this->setWindowTitle("LibVirt External Snapshots GUI Manager");

    QHBoxLayout* hAppLayout = new QHBoxLayout(this);

    m_vLColumnLayout = new QVBoxLayout();
    m_vRColumnLayout = new QVBoxLayout();

    hAppLayout->addLayout(m_vLColumnLayout);
    hAppLayout->addLayout(m_vRColumnLayout);

    m_vVmLayout = new QVBoxLayout;
    m_vVmLayout->setAlignment(Qt::AlignTop);
    m_vSnapLayout = new QVBoxLayout;

    VmDataCollector vmDataCollector;
    m_vmList = vmDataCollector.getVmList();

    m_snapTreeModel = new SnapTreeModel();

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
        this, "Подтверждение выхода", "Вы действительно хотите выйти?",
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

    QToolButton* startBtn = new QToolButton(vmBtnFrame);
    startBtn->setText("Start");
    startBtn->setFixedHeight(m_btnSize1);
    startBtn->setFixedWidth(1.5*m_btnSize1);
    startBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

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

    vmBtnFrameHLayout->addWidget(startBtn);
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
    // snapBtnFrame->setFixedWidth(0.6 * m_appWindowWidth);

    QHBoxLayout* snapBtnFrameHLayout = new QHBoxLayout(snapBtnFrame);

    QToolButton* gotoBtn = new QToolButton(snapBtnFrame);
    gotoBtn->setText("Go to");
    gotoBtn->setFixedHeight(m_btnSize1);
    gotoBtn->setFixedWidth(1.5*m_btnSize1);
    gotoBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QToolButton* deleteBtn = new QToolButton(snapBtnFrame);
    deleteBtn->setText("Delete");
    deleteBtn->setFixedHeight(m_btnSize1);
    deleteBtn->setFixedWidth(1.5*m_btnSize1);
    deleteBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QToolButton* exitBtn = new QToolButton(snapBtnFrame);
    exitBtn->setText("Exit");
    exitBtn->setFixedHeight(m_btnSize1);
    exitBtn->setFixedWidth(1.5*m_btnSize1);
    exitBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    QObject::connect(exitBtn, &QToolButton::clicked, this,&QAppWindow::appExit);

    snapBtnFrameHLayout->addWidget(gotoBtn);
    snapBtnFrameHLayout->addWidget(deleteBtn);
    snapBtnFrameHLayout->addStretch();
    snapBtnFrameHLayout->addWidget(exitBtn);

    m_vRColumnLayout->addWidget(snapBtnFrame);
}

void QAppWindow::setSnapFrame(){

    m_snapTreeView = new QTreeView();
    m_snapTreeView->header()->setStretchLastSection(false);
    m_snapTreeView->header()
                          ->setSectionResizeMode(QHeaderView::ResizeToContents);
    // m_snapTreeView->setItemsExpandable(false);
    m_snapTreeView->setAlternatingRowColors(true);
    m_snapTreeView->setRootIsDecorated(true);
    m_snapTreeView->header()->hide();
    m_snapTreeView->setStyleSheet(R"(
                                        QTreeView::item {
                                            height: 40px;
                                        }
                                    )");
    QFont font = m_snapTreeView->font();
    font.setPointSize(12);
    m_snapTreeView->setFont(font);

    m_vRColumnLayout->addWidget(m_snapTreeView);

    m_snapTreeView->setModel(m_snapTreeModel);

    QObject::connect(m_snapTreeView, &QTreeView::clicked,
                                         this, &QAppWindow::onTreeItemClicked);

//
//     QObject::connect(treeView, &QTreeView::clicked, this, [=](const QModelIndex& index) {
//     const SnapNode& node = snapTreeModel->nodeFromIndex(index);
//     if (node.id != -1) {
//         qDebug() << "Info поля узла:" << node.info;
//     }
// });

}

void QAppWindow::addVmToFrame(){
    for (int i = 0; i < m_vmList.size(); ++i){
        VmWidget* vmWidget = new VmWidget(i, m_vmList, this);
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
    }
}

void QAppWindow::updSnapTree(){
    VMachine activeVm = getActiveVm();

    qDebug() << "[II] Update snap tree now!";
    qDebug() << "[II] Selected VM index:" << m_selectedVmIndex;

    QThread* thread = new QThread;
    VmDataCollector* vmDataCollector = new VmDataCollector(activeVm, this);
    vmDataCollector->moveToThread(thread);

    thread->start();

    QObject::connect(thread, &QThread::started,
                                    vmDataCollector, &VmDataCollector::process);

    QObject::connect(vmDataCollector, &VmDataCollector::finished, this,
        [=](const VMachine& result) {
                QVector<ChainNode> vmSnapshotsChain = result.vmStateChain;
                m_snapTreeView->clearSelection();
                m_snapTreeModel->setSnapData(vmSnapshotsChain);
                m_snapTreeView->expandAll();
                m_currentVmName = result.name;
                m_mountStorages = result.mountStorages;
                for (int i = 0; i < result.vmStateChain.size(); ++i){
                   qDebug() << vmSnapshotsChain[i].id
                            << vmSnapshotsChain[i].parentId
                            << vmSnapshotsChain[i].name;
                }
                qDebug() << "[II] Данные получены (finished)";

                thread->quit();
                thread->wait();

                vmDataCollector->deleteLater();
                this->takeSnapBtnManage();
                thread->deleteLater();
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
    SnapManager* snapManager = new SnapManager(this);
    snapManager->doSnapshot(m_currentVmName, m_mountStorages);
    snapManager->deleteLater();
    // *** Обновление дерева снимков состояний виртуальной машины *** //
    updSnapTree();
}

void QAppWindow::onTreeItemClicked(const QModelIndex& index){
    QString text = index.data(Qt::DisplayRole).toString();
    qDebug() << "Клик по строке:" << text;

    const ChainNode& node = m_snapTreeModel->getChainNodeByIndex(index);
    m_activeNode = m_snapTreeModel->getChainNodeByIndex(index);
    if (node.id != -1) {
        qDebug() << "Текущий узел цепочки сохранения состояний ("
                                                        + node.imagesType + ")";
        qDebug() << "ImagesFullNames:" << node.imagesFullNames;
        qDebug() << "   BackFullName:" << node.backFullNames;
    }
}

void QAppWindow::resizeEvent(QResizeEvent* event) {
    // Fix width alternate color "bug". It's only text width
    m_snapTreeView->header()
                        ->setMinimumSectionSize(width() - m_appWindowWidth/2.3);
}

void QAppWindow::takeSnapBtnManage(){
    if (m_mountStorages.size() > 0){
        m_takeSnapBtn->setEnabled(true);
    }
    else {
        m_takeSnapBtn->setEnabled(false);
    }
}

// End appWindow.cpp
