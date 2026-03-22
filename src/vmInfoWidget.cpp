// Begin vmInfoWidget.cpp

#include "vmInfoWidget.h"
#include "nodeInfoProvider.h"
#include <QFontMetrics>

VmInfoWidget::VmInfoWidget(QWidget* parent) : QFrame(parent){

    m_saveTitleTimer = new QTimer();
    m_saveTitleTimer->setInterval(1000);
    m_saveTitleTimer->setSingleShot(true);

    m_saveDescriptionTimer = new QTimer();
    m_saveDescriptionTimer->setInterval(1000);
    m_saveDescriptionTimer->setSingleShot(true);

    QVBoxLayout* vFrameLayout = new QVBoxLayout(this);
    vFrameLayout->setAlignment(Qt::AlignTop);
    vFrameLayout->setContentsMargins(7, 5, 0, 0);

    QLabel* headline = new QLabel(this);
    QFont headlineFont = headline->font();
    int headlineOriginalFontSize = headlineFont.pointSize();
    headlineFont.setPointSize(headlineOriginalFontSize);
    headline->setFont(headlineFont);
    headline->setText("<b>Virtual machine</b>");
    vFrameLayout->addWidget(headline);

    QScrollArea* scrollForm = new QScrollArea(parent);
    scrollForm->setFrameShape(QFrame::NoFrame);
    scrollForm->setFrameShadow(QFrame::Plain);
    this->fixScrollBar(scrollForm);

    // *** Обязательно, чтобы контейнер не "схлопнулся" *** //
    scrollForm->setWidgetResizable(true);
    vFrameLayout->addWidget(scrollForm);

    // *** Виджет контейнер для правильного скролла *** //
    //       (контейнер должен быть без родителя)
    QWidget* container = new QWidget();

    m_vScrollLayout = new QVBoxLayout();
    m_vScrollLayout->setAlignment(Qt::AlignTop);
    container->setLayout(m_vScrollLayout);
    scrollForm->setWidget(container);

    QStringList colAList = {"Title:",
                            "Description:",
                            "UUID:",
                            "Canonical name:",
                            "Virtual CPUs:",
                            "RAM:",
                            "Guest OS:",
                            "Mounted drives:"};

    int colAWidth = 100;
    QVector<QHBoxLayout*> hLayoutArray;

    // *** Создание горизонтальных layout'ов и установка отсупов *** //
    for (int i = 0; i < colAList.size(); ++i){
        hLayoutArray.push_back(new QHBoxLayout);
        hLayoutArray[i]->setContentsMargins(0, 0, 0, 0);
    }

    // *** Генерация виджетов первого столбца *** //
    for (int i = 0; i < colAList.size(); ++i){
        m_colAWidgets.push_back(new QLabel(colAList[i]));
        m_colAWidgets[i]->setFixedWidth(colAWidth);
    }

    // *** Генерация виджетов второго столбца *** //
    for (int i = 0; i < colAList.size(); ++i){
        if (i == 0){ // Title
            m_colBWidgets.push_back(new QTextEdit(this));
            QWidget* widget = m_colBWidgets.last();
            QTextEdit* line = qobject_cast<QTextEdit*>(widget);
            // *** a) корректировка шрифта *** //
            QFont font = this->font();
            int newFontSize = calcOptimalFontSize(colAList);
            font.setPointSize(newFontSize);
            this->setFont(font);
            line->setFont(font);
            // *** б) установка высоты виджета *** //
            QFontMetrics fm(line->font());
            int rowHeight = fm.lineSpacing();
            int docMargin = line->document()->documentMargin();
            line->setFixedHeight(rowHeight * 1.8);
            line->viewport()->setStyleSheet("background-color: white;");
            line->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            line->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            line->setLineWrapMode(QTextEdit::NoWrap);
            line->setStyleSheet("background-color: white;");
            line->setPlaceholderText("Virtual machine title…");
            line->setReadOnly(true);
            line->installEventFilter(this);
            QObject::connect(line, &QTextEdit::textChanged,
                                    this, &VmInfoWidget::restartSaveTitleTimer);
            QObject::connect(m_saveTitleTimer, &QTimer::timeout,
                                             this, &VmInfoWidget::writeVmTitle);
            continue;
        }

        if (i == 1){ // Description
            // *** Виджет для Description должен быть многострочный *** //
            m_colBWidgets.push_back(new QTextEdit(this));
            QWidget* widget = m_colBWidgets.last();
            QTextEdit* edit = qobject_cast<QTextEdit*>(widget);
            // *** Увеличение числа отображемых строк в поле ввода до 4 *** //
            // *** a) корректировка шрифта *** //
            QFont font = this->font();
            int newFontSize = calcOptimalFontSize(colAList);
            font.setPointSize(newFontSize);
            this->setFont(font);
            edit->setFont(font);
            // *** б) Изменение числа строк *** //
            QFontMetrics fm(edit->font());
            int rowHeight = fm.lineSpacing();
            edit->setFixedHeight(rowHeight * 4.8);
            edit->viewport()->setStyleSheet("background-color: white;");
            fixScrollBar(edit);
            edit->setReadOnly(true);
            edit->setPlaceholderText("Virtual machine description…");
            QObject::connect(edit, &QTextEdit::textChanged,
                              this, &VmInfoWidget::restartSaveDescriptionTimer);
            QObject::connect(m_saveDescriptionTimer, &QTimer::timeout,
                                       this, &VmInfoWidget::writeVmDescription);
            continue;
        }

        m_colBWidgets.push_back(new QLabel(this));
        QWidget* widget = m_colBWidgets.last();
        QLabel* lbl = qobject_cast<QLabel*>(widget);
        lbl->setTextInteractionFlags(
                      Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

        // *** Присвоение значений виджетам второго столбца *** //
        switch (i){
            case 2: // UUID
                lbl->setText("—");
                break;
            case 3: // Canonical name
                lbl->setText("—");
                break;
            case 4: // CPU
                lbl->setText("—");
                break;
            case 5: // RAM
                lbl->setText("—");
                break;
            case 6: // OsName
                lbl->setText("—");
                break;
            case 7: // Mounted drives
                lbl->setText("—");
                break;
        }
    }

    // *** Добавление виджетов первого и второго столбцов в hLayout *** //
    for (int i = 0; i < m_colAWidgets.size(); ++i){
        hLayoutArray[i]->addWidget(m_colAWidgets[i]);
        hLayoutArray[i]->addWidget(m_colBWidgets[i]);
        switch (i){
            case 4: { // Add edit CPU's settings
                    m_editCpuTopologyBtn = new QToolButton();
                    m_editCpuTopologyBtn->setAutoRaise(true);
                    m_editCpuTopologyBtn->setText("⋮");
                    m_editCpuTopologyBtn
                                      ->setPopupMode(QToolButton::InstantPopup);
                    setEditBtnStyle(m_editCpuTopologyBtn);
                    m_editCpuTopologyBtn->setVisible(false);

                    QIcon actCpuTopologyIcon = QIcon(":/vmInfo-set-vm-cpu.svg");

                    QMenu* editCpuTopologyMenu =new QMenu(m_editCpuTopologyBtn);
                    QAction* actCpuTopology = editCpuTopologyMenu
                                              ->addAction("Edit CPU topology…");
                    actCpuTopology->setIcon(actCpuTopologyIcon);
                    m_editCpuTopologyBtn->setMenu(editCpuTopologyMenu);
                    hLayoutArray[i]->addWidget(m_editCpuTopologyBtn);
                    QObject::connect(actCpuTopology, &QAction::triggered,
                                            this, &VmInfoWidget::manageVmCpu,
                                                          Qt::UniqueConnection);
                    break;
                }
            case 5: { // Add edit VM RAM size
                    m_editRamSizeBtn = new QToolButton();
                    m_editRamSizeBtn->setAutoRaise(true);
                    m_editRamSizeBtn->setText("⋮");
                    m_editRamSizeBtn->setPopupMode(QToolButton::InstantPopup);
                    setEditBtnStyle(m_editRamSizeBtn);
                    m_editRamSizeBtn->setVisible(false);

                    QIcon actRamSizeIcon = QIcon(":/vmInfo-set-vm-ram.svg");

                    QMenu* editRamSizeMenu = new QMenu(m_editRamSizeBtn);
                    QAction* actRamSize = editRamSizeMenu
                                                   ->addAction("Set RAM size…");
                    actRamSize->setIcon(actRamSizeIcon);
                    m_editRamSizeBtn->setMenu(editRamSizeMenu);
                    hLayoutArray[i]->addWidget(m_editRamSizeBtn);

                    QObject::connect(actRamSize, &QAction::triggered,
                                        this, &VmInfoWidget::manageRamSize,
                                                          Qt::UniqueConnection);
                    break;
                }
            case 6: { // Add edit OS
                    m_editOsBtn = new QToolButton();
                    m_editOsBtn->setAutoRaise(true);
                    m_editOsBtn->setText("⋮");
                    m_editOsBtn->setPopupMode(QToolButton::InstantPopup);
                    setEditBtnStyle(m_editOsBtn);
                    m_editOsBtn->setVisible(false);

                    QIcon actOsIcon = QIcon(":/vmInfo-set-vm-os.svg");

                    QMenu* osMenu = new QMenu(m_editOsBtn);
                    QAction* actOs = osMenu->addAction("Manage OS info…");
                    actOs->setIcon(actOsIcon);
                    m_editOsBtn->setMenu(osMenu);
                    hLayoutArray[i]->addWidget(m_editOsBtn);

                    QObject::connect(actOs, &QAction::triggered,
                                        this, &VmInfoWidget::manageGuestOS,
                                                          Qt::UniqueConnection);
                    break;
                }
             case 7: { // Add edit mounted drives button
                    m_editImagesBtn = new QToolButton();
                    m_editImagesBtn->setAutoRaise(true);
                    m_editImagesBtn->setText("⋮");
                    m_editImagesBtn->setPopupMode(QToolButton::InstantPopup);
                    setEditBtnStyle(m_editImagesBtn);
                    m_editImagesBtn->setVisible(false);

                    QIcon actNewImgIcon = QIcon(":/vmInfo-new-drive.svg");
                    QIcon actDelImgIcon = QIcon(":/vmInfo-del-drive.svg");

                    QMenu* editImgsMenu = new QMenu(m_editImagesBtn);
                    QAction* actNewImgs = editImgsMenu
                                                   ->addAction("New VM drive…");
                    actNewImgs->setIcon(actNewImgIcon);
                    QAction* actDelImgs = editImgsMenu
                                                   ->addAction("Delete drive…");
                    actDelImgs->setIcon(actDelImgIcon);

                    m_editImagesBtn->setMenu(editImgsMenu);

                    QObject::connect(editImgsMenu , &QMenu::aboutToShow,
                                    this, &VmInfoWidget::manageDeleteItem,
                                                          Qt::UniqueConnection);

                    QObject::connect(actNewImgs, &QAction::triggered,
                                        this, &VmInfoWidget::addNewVmImages,
                                                          Qt::UniqueConnection);

                    QObject::connect(actDelImgs, &QAction::triggered,
                                              this, &VmInfoWidget::delVmImages,
                                                          Qt::UniqueConnection);

                    hLayoutArray[i]->addWidget(m_editImagesBtn);
                    break;
                }
            }
    }

    // *** Добавление hLayout'ов в основной вертикальный layout *** //
    //     Также дополнительный статичный контейнер                 //
    //     для верхних виджетов, необходимый для предотвращения     //
    //     "прыжков" виджетов при добавлении нового диска           //
    //     П.С. впрочем, проблема прыжков может быть решена через   //
    //     изменение размера виджета по таймеру, но менее надёжно.  //
    // ************************************************************ //
    QWidget*     staticContainerWidget = new QWidget();
    QVBoxLayout* staticContainerLayout = new QVBoxLayout();
    staticContainerWidget->setContentsMargins(0, 0, 0, 0);
    staticContainerLayout->setContentsMargins(0, 0, 0, 0);
    staticContainerWidget->setLayout(staticContainerLayout);
    for (int i = 0; i < hLayoutArray.size(); ++i){
        staticContainerLayout->addLayout(hLayoutArray[i]);
    }
    staticContainerWidget->setSizePolicy(QSizePolicy::Expanding,
                                                            QSizePolicy::Fixed);
    QFontMetrics fm(this->font());
    int rowHeight = fm.lineSpacing();
    staticContainerWidget->setFixedHeight(17*rowHeight);
    m_vScrollLayout->setSpacing(0.4*rowHeight);
    m_vScrollLayout->addWidget(staticContainerWidget);

    // *** Добавление и настройка виджета для отображения списка дисков *** //
    QTextEdit* storageList = new QTextEdit();

    storageList->setLineWrapMode(QTextEdit::NoWrap);
    storageList->setReadOnly(true);
    storageList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    storageList->setLineWrapMode(QTextEdit::NoWrap);
    storageList->viewport()->setStyleSheet("background-color: white;");
    storageList->setStyleSheet("QTextEdit {" "border: none;" "}");
    fixScrollBar(storageList);

    // *** Dynamic font size *** //
    QFont font = this->font();
    int newFontSize = calcOptimalFontSize(colAList);
    font.setPointSize(newFontSize);
    this->setFont(font);
    qobject_cast<QTextEdit*>(m_colBWidgets[0])->setFont(font);
    storageList->setFont(font);

    m_vScrollLayout->addWidget(storageList);
    m_vScrollLayout->addStretch();
}

VmInfoWidget::~VmInfoWidget(){
}

void VmInfoWidget::fixScrollBar(QScrollArea* scrollArea){
    // *** Костыль для скрытия квадратика при обоих скроллах *** //
    QWidget* corner = new QWidget();
    corner->setFixedSize(0, 0);
    scrollArea->setCornerWidget(corner);

    // *** Костыль для вертикального скролла *** //
    scrollArea->verticalScrollBar()->setStyleSheet(R"(
                                    QScrollBar:vertical {
                                        background: transparent;
                                        width: 6px;
                                        margin: 0px 0px 0px 0px;
                                    }
                                    QScrollBar::handle:vertical {
                                        background: #badcef;
                                        min-height: 15px;
                                        border-radius: 1px;
                                    }
                                    QScrollBar::add-line, QScrollBar::sub-line {
                                        height: 0px;
                                    }
                                    )");

    // *** Костыль для горизонтального скролла *** //
    scrollArea->horizontalScrollBar()->setStyleSheet(R"(
                                    /* Горизонтальный скроллбар */
                                     QScrollBar:horizontal {
                                        background: transparent;
                                        height: 6px;
                                        margin: 0px 0px 0px 0px;
                                     }
                                     QScrollBar::handle:horizontal {
                                        background: #badcef;
                                        min-width: 15px;
                                        border-radius: 1px;
                                     }
                                    QScrollBar::add-line, QScrollBar::sub-line {
                                        height: 0px;
                                    }
                                    )");
}

void VmInfoWidget::fixScrollBar(QTextEdit* edit){

    // *** Костыль для вертикального скролла *** //
    edit->verticalScrollBar()->setStyleSheet(R"(
                                    QScrollBar:vertical {
                                        background: transparent;
                                        width: 5px;
                                        margin: 0px 0px 0px 0px;
                                    }
                                    QScrollBar::handle:vertical {
                                        background: #badcef;
                                        min-height: 15px;
                                        border-radius: 1px;
                                    }
                                    QScrollBar::add-line, QScrollBar::sub-line {
                                        height: 0px;
                                    }
                                    )");

    // *** Костыль для горизонтального скролла *** //
    edit->horizontalScrollBar()->setStyleSheet(R"(
                                    /* Горизонтальный скроллбар */
                                     QScrollBar:horizontal {
                                        background: transparent;
                                        height: 5px;
                                        margin: 0px 0px 0px 0px;
                                     }
                                     QScrollBar::handle:horizontal {
                                        background: #badcef;
                                        min-width: 15px;
                                        border-radius: 1x;
                                     }
                                    QScrollBar::add-line, QScrollBar::sub-line {
                                        height: 0px;
                                    }
                                    )");
}

void VmInfoWidget::setData(const VMachine& vm){
    m_titleChangedCounter = 0;
    m_descriptionChangedCounter = 0;
    m_uuid = vm.uuid;
    qobject_cast<QTextEdit*>(m_colBWidgets[0])->setText(vm.title);
    qobject_cast<QTextEdit*>(m_colBWidgets[0])->setReadOnly(false);
    qobject_cast<QTextEdit*>(m_colBWidgets[1])->setText(vm.description);
    qobject_cast<QTextEdit*>(m_colBWidgets[1])->setReadOnly(false);
    qobject_cast<QLabel*>(m_colBWidgets[2])->setText(vm.uuid);
    setVmName(vm.name);
    QString vmCpuTopology = vm.cpu["topology"];
    QString vmCpuModel = vm.cpu["model"];
    QString vmMaxCPUs = vm.cpu["max"];
    qobject_cast<QLabel*>(m_colBWidgets[4])->setText(vmCpuTopology);
    qobject_cast<QLabel*>(m_colBWidgets[4])->setProperty("model", vmCpuModel);
    qobject_cast<QLabel*>(m_colBWidgets[4])->setProperty("max", vmMaxCPUs);
    QString ruHumanMemory = humanMemory(vm.ram).replace(".", ",");
    qobject_cast<QLabel*>(m_colBWidgets[5])->setText(ruHumanMemory);
    setOsName(vm.os.name);
    QString mountStoragesCount = QString::number(vm.mountStorages.size());
    qobject_cast<QLabel*>(m_colBWidgets[7])->setText(mountStoragesCount);
    setStorageList(vm.mountStorages);
    m_editImagesBtn->setVisible(true);
    m_editRamSizeBtn->setVisible(true);
    m_editCpuTopologyBtn->setVisible(true);
    m_editOsBtn->setVisible(true);

    // *** Одним отображением данных дело не обошлось, данные необходимо *** //
    //     хранить, для передачи, например, в диалог удаления хранилища      //
    m_driveBusTypes = vm.driveBusTypes;
    m_driveDevNames = vm.driveDevNames;
    m_driveVirtSizes = vm.rootVirtSizes;
    m_mountStorages = vm.mountStorages;
    m_driveChildren = vm.vmStateChain.size() - 1;
}

QString VmInfoWidget::humanMemory(const QString& rawRam){
    QString res = rawRam;

    float humanMemValue;
    QString humanMemUnits;

    // *** Пустая строка будет причиной проблем с её парсингом *** //
    if (res.size() < 3){ res = "0 GB";}
    QStringList parts = res.split(" ", Qt::SkipEmptyParts);

    long int memValue = parts.first().toLong();
    QString  memUnits = parts.last();

    // *** Передано число без единиц измерения (байты) *** //
    //     В этом случае pars.first() == parts.last()      //
    if (parts.size() == 1){
        memUnits = "bytes";
    }

    QStringList s1024BaseUnits = {   "k",   "M",   "G",   "T" };
    QStringList l1024BaseUnits = { "KiB", "MiB", "GiB", "TiB" };
    QStringList v1000BaseUnits = {  "KB",  "MB",  "GB",  "TB" };

    long int bytesRam;
    if (memUnits == "b" || memUnits == "bytes"){
        bytesRam = memValue;
    }

    if (memUnits == "KB" ){
        bytesRam = memValue * 1000;
    }

    if (memUnits == "MB"){
        bytesRam = memValue * 1000 * 1000;
    }

    if (memUnits == "GB"){
        bytesRam = memValue * 1000 * 1000 * 1000;
    }

    if (memUnits == "TB"){
        bytesRam = memValue * 1000 * 1000 * 1000 * 1000;
    }

    if (memUnits == "k" || memUnits == "KiB"){
        bytesRam = memValue * 1024;
    }

    if (memUnits == "M" || memUnits == "MiB"){
        bytesRam == memValue * 1024 * 1024;
    }

    if (memUnits == "G" || memUnits == "GiB"){
        bytesRam == memValue * 1024 * 1024 * 1024;
    }

    if (memUnits == "T" || memUnits == "TiB"){
        bytesRam == memValue * 1024 * 1024 * 1024 * 1024;
    }

    humanMemValue = bytesRam;
    for (int i = 0; i < l1024BaseUnits.size(); ++i){
        humanMemValue = humanMemValue / 1024.0;
        if (humanMemValue < 1024){
            humanMemUnits = l1024BaseUnits[i];
            break;
        }
    }

    if (abs(humanMemValue - (int)humanMemValue) < 0.01){
        res = QString::number(humanMemValue, 'f', 0) + " " + humanMemUnits;
    } else {
        QString number = QString::number(humanMemValue, 'f', 2);

        // *** Удалить последний ноль в размере *** //
        while (number.contains('.') && number.endsWith('0')){
            number.chop(1);
        }

        if (number.endsWith('.')){
            number.chop(1);
        }

        res = number + " " + humanMemUnits;
    }

    return res;
}

void VmInfoWidget::setStorageList(const QStringList& mountStorages){
    // *** Изменение локальных (для vmInfoWidget) данных *** //
    m_mountStorages = mountStorages;

    QStringList listData;

    for (int i = 0; i < mountStorages.size(); ++i){
        listData.push_back(QString::number(i+1) + ". " + mountStorages[i]);
    }

    // *** Доcтуп к виджету списка примонтированных дисков *** //
    QWidget* editWidget =  m_vScrollLayout
                               ->itemAt(m_vScrollLayout->count() - 2)->widget();
    QTextEdit* storageList = qobject_cast<QTextEdit*>(editWidget);

    // *** Изменение его вертикального размера для вмещения всех дисков *** //
    QFontMetrics fm(storageList->font());
    int rowHeight = fm.lineSpacing();
    int imagesCount = listData.size();
    int docMargin = storageList->document()->documentMargin();
    int newHeight = rowHeight * imagesCount + docMargin + 12;

    storageList->setFixedHeight(newHeight);

    storageList->setText(listData.join("\n"));

    // *** Обновление числа примонтированных дисков (Mounted drives) *** //
    qobject_cast<QLabel*>(m_colBWidgets[7])
                                    ->setText(QString::number(listData.size()));
}

void VmInfoWidget::setDriveBusTypeList(const QStringList& driveBusTypes){
    m_driveBusTypes = driveBusTypes;
}

void VmInfoWidget::setDriveDevNameList(const QStringList& driveDevNames){
    m_driveDevNames = driveDevNames;
}

void VmInfoWidget::setDriveVirtSizeList(const QStringList& driveVirtSizes){
    m_driveVirtSizes = driveVirtSizes;
}

void VmInfoWidget::setDriveChildren(const unsigned int& driveChildren){
    m_driveChildren = driveChildren;
}

void VmInfoWidget::writeVmTitle(){
    QString vm_uuid = m_uuid;
    QString vm_titl = qobject_cast<QTextEdit*>(m_colBWidgets[0])->toPlainText();
    emit writeVmTitleRequested(vm_uuid, vm_titl);
    // *** Отправка сигнала, что ввод текста завершен *** //
    emit textChangedEnd();
}

void VmInfoWidget::writeVmDescription(){

    QString vm_uuid = m_uuid;
    QString vm_desc = qobject_cast<QTextEdit*>(m_colBWidgets[1])->toPlainText();
    emit writeVmDescriptionRequested(vm_uuid, vm_desc);

    // *** Отправка сигнала, что ввод текста завершен *** //
    emit textChangedEnd();
}

void VmInfoWidget::restartSaveTitleTimer(){
    // *** Первые события - создание QTextEdit и присвоение через setData *** //
    if (m_titleChangedCounter < 10){
        m_titleChangedCounter++;
        if (m_titleChangedCounter < 2){
            return;
        }
    }

    m_saveTitleTimer->start();
    emit textChangedBegin();
}

void VmInfoWidget::restartSaveDescriptionTimer(){

    if (m_descriptionChangedCounter < 10){
        m_descriptionChangedCounter++;
        if (m_descriptionChangedCounter < 2){
            return;
        }
    }

    m_saveDescriptionTimer->start();
    emit textChangedBegin();
}

bool VmInfoWidget::eventFilter(QObject *obj, QEvent* event){
    QWidget*   widget = m_colBWidgets[0];
    QTextEdit* line = qobject_cast<QTextEdit*>(widget);
    if (obj == line && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Return
                                          || keyEvent->key() == Qt::Key_Enter) {
                return true;
            }
            else {
                if (keyEvent->key() == Qt::Key_Tab){
                    QTextEdit* edt = qobject_cast<QTextEdit*>(m_colBWidgets[1]);
                    edt->setFocus();
                    edt->moveCursor(QTextCursor::End);
                    return true;
                }
            }
        }
        return QWidget::eventFilter(obj, event);
}

VMachine VmInfoWidget::getData(){
    VMachine vm;

    vm.title = qobject_cast<QTextEdit*>(m_colBWidgets[0])->toPlainText();
    vm.description = qobject_cast<QTextEdit*>(m_colBWidgets[1])->toPlainText();
    vm.uuid = qobject_cast<QLabel*>(m_colBWidgets[2])->text();
    vm.name = getVmName();
    vm.cpu["topology"] = qobject_cast<QLabel*>(m_colBWidgets[4])->text();
    vm.cpu["model"] = qobject_cast<QLabel*>(m_colBWidgets[4])
                                                 ->property("model").toString();
    vm.cpu["max"] = qobject_cast<QLabel*>(m_colBWidgets[4])
                                                   ->property("max").toString();
    vm.ram = qobject_cast<QLabel*>(m_colBWidgets[5])->text();
    vm.os.name = getVmOS();

    return vm;
}

void VmInfoWidget::setTitle(const QString& newTitle){
    m_titleChangedCounter = 0;
    QTextEdit* edit = qobject_cast<QTextEdit*>(m_colBWidgets[0]);
    // *** Применение изменений тогда, когда курсора нет в поле ввода *** //
    if (!edit->hasFocus()){
        edit->setText(newTitle);
    }
}

void VmInfoWidget::setDescription(const QString& newDescription){
    m_descriptionChangedCounter = 0;
    QTextEdit* edit = qobject_cast<QTextEdit*>(m_colBWidgets[1]);
    if (!edit->hasFocus()){
        edit->setText(newDescription);
    }
}

void VmInfoWidget::setCpuTopology(const QString& newCpuTopology){
    qobject_cast<QLabel*>(m_colBWidgets[4])->setText(newCpuTopology);
}

void VmInfoWidget::setCpuModel(const QString& newCpuModel){
    qobject_cast<QLabel*>(m_colBWidgets[4])->setProperty("model", newCpuModel);
}

void VmInfoWidget::setMaxCPUs(const QString& newMaxCPUs){
    qobject_cast<QLabel*>(m_colBWidgets[4])->setProperty("max", newMaxCPUs);
}

void VmInfoWidget::setRam(const QString& newRam){
    QString ruHumanMemory = humanMemory(newRam).replace(".", ",");
    qobject_cast<QLabel*>(m_colBWidgets[5])->setText(ruHumanMemory);
}

void VmInfoWidget::setOsName(const QString& newOsName){
    QLabel* osNameLabel = qobject_cast<QLabel*>(m_colBWidgets[6]);
    this->setTextToLabel(osNameLabel, newOsName);
}

void VmInfoWidget::setVmName(const QString& vmName){
    QLabel* osVmLabel = qobject_cast<QLabel*>(m_colBWidgets[3]);
    this->setTextToLabel(osVmLabel, vmName);
}

void VmInfoWidget::setTextToLabel(QLabel* oLabel, const QString& inText){
    // *** Расчёт максимальной ширины текста в пикселях следующий: *** //
    //     maxTextWidth = uuidWidth - 2*fontSize т.е.,                 //
    //     максимальная ширина текста равна ширине uuid ВМ,            //
    //     (максимально длинная строка) минус ширина двух символов     //

    QLabel* uuidLabel = qobject_cast<QLabel*>(m_colBWidgets[2]);

    QFont infoFont = oLabel->font();

    uint uuidWidth = uuidLabel->width();
    uint fontSize = infoFont.pointSize();
    uint maxTextWidth = uuidWidth - 2*fontSize;

    QFontMetrics fm(oLabel->font());
    int inTextWidth = fm.boundingRect(inText).width();

    QString oText = inText;
    if (inTextWidth > maxTextWidth){
        // *** В QLabel название сокращённое, а в tooltip'е - полное *** //
        oLabel->setToolTip(inText);

        // *** elidedText() возвращает строку, которая гарантированно *** //
        //     влезет в заданную ширину, а сли не влезает — добавляет ... //
        oText = fm.elidedText( inText, Qt::ElideRight, maxTextWidth);
    }
    else {
        // *** Если нет необходимости сокращать название, tooltip выкл. *** //
        if (!oLabel->toolTip().isEmpty()){
            oLabel->setToolTip(QString());
        }
    }

    oLabel->setText(oText);
}

void VmInfoWidget::setReadOnly(bool roState){
    QTextEdit* title = qobject_cast<QTextEdit*>(m_colBWidgets[0]);
    QTextEdit* description = qobject_cast<QTextEdit*>(m_colBWidgets[1]);

    title->setReadOnly(roState);
    description->setReadOnly(roState);
    m_editImagesBtn->setEnabled(!roState);
    m_editRamSizeBtn->setEnabled(!roState);
    m_editCpuTopologyBtn->setEnabled(!roState);
    m_editOsBtn->setEnabled(!roState);

    QString roToolTip = "Can edit only when VM is shut off";
    QString blankToolTip = "";

    if (roState){
        title->setToolTip(roToolTip);
        description->setToolTip(roToolTip);
    }
    else {
        title->setToolTip(blankToolTip);
        description->setToolTip(blankToolTip);
    }
}

void VmInfoWidget::clearData(){
    m_titleChangedCounter = 0;
    m_descriptionChangedCounter = 0;
    m_uuid = QString("");
    qobject_cast<QTextEdit*>(m_colBWidgets[0])->setText("");
    qobject_cast<QTextEdit*>(m_colBWidgets[0])->setReadOnly(true);
    qobject_cast<QTextEdit*>(m_colBWidgets[1])->setText("");
    qobject_cast<QTextEdit*>(m_colBWidgets[1])->setReadOnly(true);
    qobject_cast<QLabel*>(m_colBWidgets[2])->setText("—");
    qobject_cast<QLabel*>(m_colBWidgets[3])->setText("—");
    qobject_cast<QLabel*>(m_colBWidgets[4])->setText("—");
    qobject_cast<QLabel*>(m_colBWidgets[5])->setText("—");
    qobject_cast<QLabel*>(m_colBWidgets[6])->setText("—");
    qobject_cast<QLabel*>(m_colBWidgets[7])->setText("—");
    setStorageList(QStringList());
}

int VmInfoWidget::calcOptimalFontSize(const QStringList& text){
    int res;
    int maxColAWidgetWidth = 0;
    int maxTextSize = 0;
    QString maxLengthText;

    QFont f = this->font();
    res = f.pointSize();
    QFontMetrics fm(f);

    for (int i = 2; i < m_colAWidgets.size(); ++i){
        int widgetWidth = qobject_cast<QLabel*>(m_colAWidgets[i])->width();
        int textSize = qobject_cast<QLabel*>(m_colAWidgets[i])->text().size();

        if (maxColAWidgetWidth < widgetWidth){
            maxColAWidgetWidth = widgetWidth;
        }

        if (maxTextSize < textSize){
           maxTextSize = textSize;
           maxLengthText = qobject_cast<QLabel*>(m_colAWidgets[i])->text();
        }
    }

    while (fm.horizontalAdvance(maxLengthText) > maxColAWidgetWidth
                                                          && f.pointSize() > 6){
        --res;
        f.setPointSize(res);
        fm = QFontMetrics(f);
    }

    return res;
}

void VmInfoWidget::manageVmCpu(){
    // *** Получение информации о числе логических процессооров на хосте *** //
    NodeInfoProvider* nodeInfoProvider = new NodeInfoProvider();
    uint hostLogicalCpuCount = nodeInfoProvider->logicalCpuCount();
    QString vmCpuTopology = this->getVmCpuTopology();
    QString vmCpuModel = this->getVmCpuModel();
    QString vmMaxCPUs = this->getVmMaxCPUs();

    DialogManageCpu manageCpuTopologyDialog(hostLogicalCpuCount, vmCpuModel,
                                                vmMaxCPUs, vmCpuTopology, this);
    QObject::connect(&manageCpuTopologyDialog,
                        &DialogManageCpu::requestVmCpuInfoXmlUpdate,
                              this, &VmInfoWidget::onRequestVmCpuInfoXmlUpdate);

    manageCpuTopologyDialog.exec();
}

void VmInfoWidget::manageRamSize(){

    // *** Сбор данных, необходимых для диалога оперативной памяти *** //
    QString osName = getVmOS();
    // *** VmInfoWidget не имеет информации о минимальном рекомендуемом *** //
    //     значении памяти для текущей операционной системы. Получаем       //
    //     через OsInfoProvider'а                                           //
    OsInfoProvider osInfoProvider = new OsInfoProvider();
    OsInfo osInfo  = osInfoProvider.getOsInfoByOsName(osName);
    QString vmMinRam = humanMemory(osInfo.ram);
    QString vmRam = getData().ram;

    // *** VmInfoWidget, да, и всё отсальное не имеет информации о хосте *** //
    //     Создание провайдера данных и получение их от него                 //
    NodeInfoProvider* nodeInfoProvider = new NodeInfoProvider();
    long int hostRamValue = nodeInfoProvider->memorySizeBytes();
    QString hostRam = humanMemory(QString::number(hostRamValue));
    DialogManageRam manageRamSizeDialog(osName, vmRam, vmMinRam, hostRam, this);
    manageRamSizeDialog.setHostRamInBytes(hostRamValue);

    connect(&manageRamSizeDialog, &DialogManageRam::requestVmRamXmlUpdate,
                                  this, &VmInfoWidget::onRequestVmRamXmlUpdate);

    manageRamSizeDialog.exec();
}

void VmInfoWidget::manageGuestOS(){

    QString currentGuestOS = getVmOS();
    DialogManageGuestOS manageGuestOSDialog(currentGuestOS, this);
    // *** Переиспускание сигналов от диалога *** //
    connect(&manageGuestOSDialog, &DialogManageGuestOS::requestVmOsInfoUpdate,
                                  this, &VmInfoWidget::onRequestVmOsInfoUpdate);
    connect(&manageGuestOSDialog,&DialogManageGuestOS::requestVmOsXmlInfoUpdate,
                               this, &VmInfoWidget::onRequestVmOsXmlInfoUpdate);
    connect(&manageGuestOSDialog,&DialogManageGuestOS::requestVmOsXmlInfoClear,
                                this, &VmInfoWidget::onRequestVmOsXmlInfoClear);

    // ***   Запуск диалога установки гостевой ОС   *** //
    //     accept || reject не важно т.к., все действия //
    //     реализованы через сигналы/слоты (см. выше).  //
    manageGuestOSDialog.exec();
}

void VmInfoWidget::addNewVmImages(){
    DialogAddNewImage addNewImageDialog(this);

    if (addNewImageDialog.exec() == QDialog::Accepted){
        QString imageFullName = addNewImageDialog.getImageFullName();
        QString imageSize     = addNewImageDialog.getImageSize();
        QString imageBusType  = addNewImageDialog.getImageBusType();
        emit addNewVmImagesRequested({imageFullName, imageSize, imageBusType});
    }
}

void VmInfoWidget::delVmImages(){
    DialogDeleteImage deleteImageDialog(this);

    // *** Установка параметров для отображения в диалоге *** //
    deleteImageDialog.setRootStorageList(m_mountStorages);
    deleteImageDialog.setDriveBusTypeList(m_driveBusTypes);
    deleteImageDialog.setDriveDevNameList(m_driveDevNames);
    deleteImageDialog.setDriveVirtSizeList(m_driveVirtSizes);
    deleteImageDialog.setDriveChildren(m_driveChildren);

    if (deleteImageDialog.exec() == QDialog::Accepted){
        unsigned int idx = deleteImageDialog.getDeleteImagesIndex();
        emit delVmImagesRequested(idx);
    }
}

void VmInfoWidget::manageDeleteItem(){

    // *** Второй пункт меню - пункт удаления. *** //
    QAction* delImgAction = m_editImagesBtn->menu()->actions().at(1);
    if (m_mountStorages.size() > 0){
        if (!delImgAction->isEnabled()){
            delImgAction->setEnabled(true);
        }
    }
    else {
        if (delImgAction->isEnabled()){
            delImgAction->setEnabled(false);
        }
    }
}

void VmInfoWidget::setEditBtnStyle(QToolButton* editBtn){

    QFontMetrics fm(this->font());
    int lineSize = fm.lineSpacing();
    editBtn->setFixedHeight(lineSize);
    editBtn->setFixedWidth(lineSize);
    editBtn->setStyleSheet(R"(
            QToolButton:hover {
                border: 1px solid #0078d7;
                border-radius: 1px;
                background-color: rgba(0, 120, 215, 30%);
            }
            QToolButton:pressed {
                border: 1px solid #0078d7;
                border-radius: 1px;
                background-color: rgba(0, 120, 215, 40%);
            }
            QToolButton::menu-indicator {
                image:none;
            })");
    QFont btnFont = editBtn->font();
    btnFont.setWeight(QFont::Bold);
    editBtn->setFont(btnFont);
}

QString VmInfoWidget::getFullTextFromLabel(QLabel* label){
    QString res;

    // *** При необходимости сокращеня названия в toolTip содержится *** //
    //     его полная версия, без сокращения toolTip пустой.             //
    if (label->toolTip().size() > 0){
        res = label->toolTip();
    }
    else {
        res = label->text();
    }

    return res;
}

QString VmInfoWidget::getVmOS(){
    QWidget* widget = m_colBWidgets[6];
    QLabel* guestOsLabel = qobject_cast<QLabel*>(widget);

    return getFullTextFromLabel(guestOsLabel);
}

QString VmInfoWidget::getVmName(){
    QWidget* widget = m_colBWidgets[3];
    QLabel* vmNameLabel = qobject_cast<QLabel*>(widget);

    return getFullTextFromLabel(vmNameLabel);
}


QString VmInfoWidget::getVmCpuTopology(){
    QString cpuTopology;
    // *** Стока cpuTopology вида: "sockets 2 · cores 1 · threads 1" *** //
    cpuTopology = qobject_cast<QLabel*>(m_colBWidgets[4])->text();
    return cpuTopology;
}

QString VmInfoWidget::getVmCpuModel(){
    QString cpuModel;
    cpuModel = qobject_cast<QLabel*>(m_colBWidgets[4])
                                                 ->property("model").toString();
    return cpuModel;
}

QString VmInfoWidget::getVmMaxCPUs(){
    QString vmMaxCPUs;
    vmMaxCPUs = qobject_cast<QLabel*>(m_colBWidgets[4])
                                                   ->property("max").toString();
    return vmMaxCPUs;
}

void VmInfoWidget::onRequestVmOsInfoUpdate(){
    // *** m_vmInfoWidget посылает сигнал о необходимости обновить БД *** //
    //     m_vmDataCollector должен отреагировать на данный сигнал.       //
    emit requestVmOsInfoUpdate();
}

void VmInfoWidget::onRequestVmOsXmlInfoClear(){
    // *** переиспускание сигнала от диалога *** //
    emit requestVmOsXmlInfoClear();
}

void VmInfoWidget::onRequestVmOsXmlInfoUpdate(const OsInfo& osInfo){
    // *** переиспускание сигнала от диалога *** //
    emit requestVmOsXmlInfoUpdate(osInfo);
}

void VmInfoWidget::onRequestVmRamXmlUpdate(const long int& memoryInKiB){
    // *** переиспускание сигнала от диалога *** //
    emit requestVmRamXmlUpdate(memoryInKiB);
}

void VmInfoWidget::onRequestVmCpuInfoXmlUpdate(const uint& sockets,
                                         const uint& cores,
                                         const uint& threads,
                                         const QString vmCpuModel){
    // *** переиспускание сигнала от диалога "dialogManageCpu.cpp" *** //
    emit requestVmCpuInfoXmlUpdate(sockets, cores, threads, vmCpuModel);
}
// End vmInfoWidget.cpp
