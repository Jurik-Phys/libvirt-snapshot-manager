// Begin vmInfoWidget.cpp

#include "vmInfoWidget.h"

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
                    break;
                }
            case 6: { // Add edit OS
                    m_editOsBtn = new QToolButton();
                    m_editOsBtn->setAutoRaise(true);
                    m_editOsBtn->setText("⋮");
                    m_editOsBtn->setPopupMode(QToolButton::InstantPopup);
                    setEditBtnStyle(m_editOsBtn);
                    m_editOsBtn->setVisible(false);

                    QIcon actOsBtnIcon = QIcon(":/vmInfo-set-vm-os.svg");

                    QMenu* osBtnMenu = new QMenu(m_editOsBtn);
                    QAction* actOsBtn = osBtnMenu
                                                ->addAction("Select guest OS…");
                    actOsBtn->setIcon(actOsBtnIcon);
                    m_editOsBtn->setMenu(osBtnMenu);
                    hLayoutArray[i]->addWidget(m_editOsBtn);
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
    qobject_cast<QLabel*>(m_colBWidgets[3])->setText(vm.name);
    qobject_cast<QLabel*>(m_colBWidgets[4])->setText(vm.cpu);
    qobject_cast<QLabel*>(m_colBWidgets[5])->setText(humanMemory(vm.ram));
    QLabel* osNameLbl = qobject_cast<QLabel*>(m_colBWidgets[6]);
    // *** Tooltip при ручном переключении VM (ручное обновление данных *** //
    if (vm.os.name.size() > vm.uuid.size()){
        osNameLbl->setToolTip(vm.os.name);
    }
    else {
        osNameLbl->setToolTip("");
    }
    osNameLbl->setText(cutLongOsName(vm.os.name, vm.uuid.size()));
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

    long int     memValue = parts[0].toLong();
    QString memUnits = parts[1];

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
        res = QString::number(humanMemValue, 'f', 2) + " " + humanMemUnits;
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
    vm.name = qobject_cast<QLabel*>(m_colBWidgets[3])->text();
    vm.cpu = qobject_cast<QLabel*>(m_colBWidgets[4])->text();
    vm.ram = qobject_cast<QLabel*>(m_colBWidgets[5])->text();
    vm.os.name = cutLongOsName(qobject_cast<QLabel*>(m_colBWidgets[6])->text(),
                                                                vm.uuid.size());
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

void VmInfoWidget::setName(const QString& newName){
    qobject_cast<QLabel*>(m_colBWidgets[3])->setText(newName);
}

void VmInfoWidget::setCpu(const QString& newCpu){
    qobject_cast<QLabel*>(m_colBWidgets[4])->setText(newCpu);
}
void VmInfoWidget::setRam(const QString& newRam){
    qobject_cast<QLabel*>(m_colBWidgets[5])->setText(humanMemory(newRam));
}

void VmInfoWidget::setOsName(const QString& newOsName){
    // *** Проверка случая для названия, которое было сокращено *** ///
    int maxTextLength = qobject_cast<QLabel*>(m_colBWidgets[2])->text().size();
    QString oldOsName = qobject_cast<QLabel*>(m_colBWidgets[6])->text();
    QString cutOsNewName = cutLongOsName(newOsName, maxTextLength);
    if (cutOsNewName != oldOsName){
        qobject_cast<QLabel*>(m_colBWidgets[6])->setText(cutOsNewName);
        if (cutOsNewName != newOsName){
            qobject_cast<QLabel*>(m_colBWidgets[6])->setToolTip(newOsName);
        }
        else {
            qobject_cast<QLabel*>(m_colBWidgets[6])->setToolTip("");
        }
    }
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

QString VmInfoWidget::cutLongOsName(const QString& longOsName,
                                                    const int& maxTextLength){
    QString osName;
    if (longOsName.size() > maxTextLength){
        osName = longOsName;
        if (osName.contains("N/A")){
            osName.chop(longOsName.size() - maxTextLength + 4);
            osName = osName + "…]";
        }
        else {
            osName.chop(longOsName.size() - maxTextLength + 3);
            osName = osName + "…";
        }
    }
    else {
        osName = longOsName;
    }

    return osName;
}
// End vmInfoWidget.cpp
