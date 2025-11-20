// Begin dialogs.cpp

#include "dialogs.h"

DialogAddNewImage::DialogAddNewImage(QWidget* parent) : QDialog(parent){
    setWindowTitle("New VM drive (base & children qcow2 images)");
    setModal(true);
    setMinimumWidth(m_leftWidth*5.2);

    QVBoxLayout* vDialogLayout = new QVBoxLayout();
    vDialogLayout->setAlignment(Qt::AlignTop);

    // *** Directory *** //
    QHBoxLayout* hDirPathLayout = new QHBoxLayout();
    QLabel* dirPathLabel = new QLabel("Directory:");
    dirPathLabel->setFixedWidth(m_leftWidth);
    m_dirPathEdit = new QLineEdit();
    m_dirPathEdit->setText("/var/lib/libvirt/images");
    QToolButton* dirPathBrowseBtn = new QToolButton();
    dirPathBrowseBtn->setText("Browse…");
    hDirPathLayout->addWidget(dirPathLabel);
    hDirPathLayout->addWidget(m_dirPathEdit);
    hDirPathLayout->addWidget(dirPathBrowseBtn);
    QObject::connect(dirPathBrowseBtn, &QToolButton::clicked,
                                           this, &DialogAddNewImage::selectDir);
    vDialogLayout->addLayout(hDirPathLayout);

    // *** File name *** //
    QHBoxLayout* hFileNameLayout = new QHBoxLayout();
    QLabel* fileNameLabel = new QLabel("File name:");
    fileNameLabel->setFixedWidth(m_leftWidth);
    m_fileNameEdit = new QLineEdit();
    m_fileNameEdit->
        setPlaceholderText("Enter image filename (e.g. vmName-volName.qcow2)");
    hFileNameLayout->addWidget(fileNameLabel);
    hFileNameLayout->addWidget(m_fileNameEdit);
    vDialogLayout->addLayout(hFileNameLayout);

    // *** Capacity *** //
    QHBoxLayout* hCapacityLayout = new QHBoxLayout();
    QLabel* capacityLabel = new QLabel("Capacity:");
    capacityLabel->setFixedWidth(m_leftWidth);
    m_capacitySpinBox = new QDoubleSpinBox();
    m_capacitySpinBox->setRange(0.0, 999999.9);
    m_capacitySpinBox->setValue(256.0);
    m_capacitySpinBox->setSingleStep(0.1);
    m_capacitySpinBox->setDecimals(1);
    m_capacitySpinBox->setSuffix(" GiB");
    m_capacitySpinBox->setFixedWidth(qRound(m_leftWidth * 1.2));
    hCapacityLayout->addWidget(capacityLabel);
    hCapacityLayout->addWidget(m_capacitySpinBox);
    hCapacityLayout->addStretch();
    m_errorOut = new QLabel();
    m_errorOut->setStyleSheet("font-weight: bold; color: red;");
    hCapacityLayout->addWidget(m_errorOut);
    vDialogLayout->addLayout(hCapacityLayout);

    // *** Controller & Error output *** //
    QHBoxLayout* hControllerLayout = new QHBoxLayout();
    QLabel* controllerLabel = new QLabel("Bus type:");
    controllerLabel->setFixedWidth(m_leftWidth);
    m_controllerBox = new QComboBox();
    m_controllerBox->addItem("VirtIO [vdX]");
    m_controllerBox->addItem("SCSI   [sdY]");
    m_controllerBox->addItem("SATA   [sdZ]");
    m_controllerBox->setFixedWidth(qRound(m_leftWidth * 1.2));
    hControllerLayout->addWidget(controllerLabel);
    hControllerLayout->addWidget(m_controllerBox);

    // *** Dialog buttons *** //
    QDialogButtonBox* dlgBtnBox = new QDialogButtonBox(
                                                QDialogButtonBox::Ok |
                                                    QDialogButtonBox::Cancel);
    hControllerLayout->addWidget(dlgBtnBox);
    vDialogLayout->addLayout(hControllerLayout);

    QObject::connect(dlgBtnBox, &QDialogButtonBox::accepted,
                                   this, &DialogAddNewImage::checkSelectedPath);
    QObject::connect(dlgBtnBox, &QDialogButtonBox::rejected,
                                              this, &DialogAddNewImage::reject);
    vDialogLayout->addWidget(dlgBtnBox);

    // *** Apply vDialogLayout *** //
    setLayout(vDialogLayout);

    // *** Fix vertical window space for QDialogButtonBox *** //
    QLayoutItem* item = vDialogLayout->takeAt(vDialogLayout->count() - 1);
    if (item->widget()) {
        item->widget()->deleteLater();
    }
    delete item;
}

DialogAddNewImage::~DialogAddNewImage(){
}

void DialogAddNewImage::checkSelectedPath(){
    m_errorOut->clear();
    // *** Проверка корректности выбранного каталога *** //
    QString imageBasePath = m_dirPathEdit->text().trimmed();
    if (!isValidDirPath(imageBasePath)){
        m_errorOut->setText("Wrong directory. ");
        return;
    }
    else {
        // *** Hack for remove multiple symbols "/" on end of path *** //
        QDir rmDouble(imageBasePath);
        imageBasePath = rmDouble.absolutePath();
    }

    // *** Проверка на существование выбранного каталога *** //
    //      Если каталога нет, то попытка его создания       //
    QDir dir(imageBasePath);
    if (!dir.exists()){
        if (!dir.mkdir(imageBasePath)){
            m_errorOut->setText("Error creating directory. ");
            return;
        }
    }

    // *** Проверка возможности записи в выбранный каталог *** //
    QTemporaryFile createTestFile(imageBasePath + "/.dirWriteTest-XXXXXX");
    if (createTestFile.open()){
        createTestFile.remove();
    }
    else {
        m_errorOut->setText("Directory write access denied!");
        return;
    }

    // *** Проверка корректности имени файла *** //
    QString fName = m_fileNameEdit->text().trimmed();
    if (!isValidFileName(fName)){
        if (fName.size() < 1){
            m_errorOut->setText("No image filename!");
        }
        else {
            m_errorOut->setText("Invalid filename!");
        }
        return;
    }
    else {
        if (!fName.endsWith(".qcow2", Qt::CaseInsensitive)){
            fName += ".qcow2";
        }
    }

    // *** Проверка на существование файла *** //
    QFile imageFile(imageBasePath + "/" + fName);
    if (imageFile.exists()){
        m_errorOut->setText("The file already exists!");
        m_fileNameEdit->setText(fName);
        return;
    }

    // *** Все проверки пройдены успешно *** //
    //     Установка причёсанного текста     //
    //     в виджеты, вызов метода accept(); //
    m_fileNameEdit->setText(fName);
    m_dirPathEdit->setText(imageBasePath);
    this->accept();
}

bool DialogAddNewImage::isValidFileName(const QString& fName){
    bool res = true;

    if (fName.isEmpty())
        res = false;

    static const QRegularExpression forbidden(R"([/\\:*?"<>|])");
    static const QRegularExpression id(R"(-id-(\d{10}))");
    if (forbidden.match(fName).hasMatch() || id.match(fName).hasMatch())
        res = false;

    if (fName.endsWith('.'))
        res = false;

    return res;
}

bool DialogAddNewImage::isValidDirPath(const QString& dir){
    bool res = true;

     if (dir.isEmpty())
        res = false;

    static const QRegularExpression forbidden(R"([*?"<>|])");
    if (forbidden.match(dir).hasMatch())
        res = false;

    if (dir.endsWith(' ') || dir.endsWith('.'))
        res = false;

    QDir tmpDir(dir);
    if (!tmpDir.isAbsolute()){
        res = false;
    }

    return res;
}

void DialogAddNewImage::selectDir(){
    QString dir = QFileDialog::getExistingDirectory(
                this,
                "Select Folder",
                "/var/lib/libvirt/images",
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    m_dirPathEdit->setText(dir);
}

QString DialogAddNewImage::getImageFullName(){
    return m_dirPathEdit->text() + "/" + m_fileNameEdit->text();
}

QString DialogAddNewImage::getImageSize(){
    return m_capacitySpinBox->cleanText().replace(",",".");
}

QString DialogAddNewImage::getImageBusType(){
    QString res;

    if (m_controllerBox->currentText() == "VirtIO [vdX]"){
        res = "VirtIO";
    }
    if (m_controllerBox->currentText() == "SCSI   [sdY]"){
        res = "SCSI";
    }
    if (m_controllerBox->currentText() == "SATA   [sdZ]"){
        res = "SATA";
    }

    return res;
}

DialogDeleteImage::DialogDeleteImage(QWidget* parent) : QDialog(parent){
    // *** parent - VmInfoWidget *** //
    this->setFixedWidth(1.7 *  parent->width());
    this->setWindowTitle("Delete VM drive (base & children qcow2 images)");

    // *** Dialog main vertical layout *** //
    QVBoxLayout* dlgVBoxLayout = new QVBoxLayout(this);

    QFrame* dlgTitleFrame = new QFrame(this);
    dlgTitleFrame->setFrameShape(QFrame::StyledPanel);
    dlgTitleFrame->setFrameShadow(QFrame::Plain);
    dlgTitleFrame->setFixedHeight(m_dlgTitleFrameHeight);
    dlgTitleFrame->setFixedWidth(1.7 * parent->width() - 22);

    QHBoxLayout* dlgTitleFrameHLayout = new QHBoxLayout(dlgTitleFrame);
    dlgTitleFrameHLayout->setAlignment(Qt::AlignHCenter);
    QLabel* dlgText = new QLabel("Select the VM drive to delete "
                                     "and confirm it using the checkbox below");
    dlgTitleFrameHLayout->addWidget(dlgText);

    m_tableDataModel = new QStandardItemModel(this);
    m_tableDataModel->setColumnCount(5);
    m_tableDataModel->setHorizontalHeaderLabels({"№", "Bus type", "Device name",
                                            "Virtual size", "Base image name"});

    m_tableDataView = new QTableView(this);
    m_tableDataView->setModel(m_tableDataModel);

    m_tableDataView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableDataView->verticalHeader()->setVisible(false);
    m_tableDataView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableDataView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableDataView->horizontalHeader()
                        ->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    m_tableDataView->horizontalHeader()
                                 ->setSectionResizeMode(4,QHeaderView::Stretch);

    QItemSelectionModel* selection = m_tableDataView->selectionModel();
    QObject::connect(selection, &QItemSelectionModel::selectionChanged,
                                           this, &DialogDeleteImage::rowSelect);

    QFrame* dlgBtnFrame = new QFrame(this);
    dlgBtnFrame->setFrameShape(QFrame::StyledPanel);
    dlgBtnFrame->setFrameShadow(QFrame::Plain);
    dlgBtnFrame->setFixedHeight(m_dlgTitleFrameHeight);
    dlgBtnFrame->setFixedWidth(1.7 * parent->width() - 22);

    QHBoxLayout* dlgBtnFrameHLayout = new QHBoxLayout(dlgBtnFrame);
    m_rmConfirmation = new QCheckBox();
    m_rmConfirmation->setEnabled(false);
    dlgBtnFrameHLayout->addWidget(m_rmConfirmation);
    m_rmConfirmation->setText("Delete confirmation checkbox");
    QObject::connect(m_rmConfirmation, &QCheckBox::checkStateChanged,
                             this, &DialogDeleteImage::deletePermissionChanged);

    dlgBtnFrameHLayout->addStretch();

    m_dlgBtnBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                      QDialogButtonBox::Cancel);
    QPushButton* okButton = m_dlgBtnBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(false);

    dlgBtnFrameHLayout->addWidget(m_dlgBtnBox);

    QObject::connect(m_dlgBtnBox, &QDialogButtonBox::accepted,
                                            this, &DialogDeleteImage::doDelete);
    QObject::connect(m_dlgBtnBox, &QDialogButtonBox::rejected,
                                              this, &DialogDeleteImage::reject);

    dlgVBoxLayout->addWidget(dlgTitleFrame);
    dlgVBoxLayout->addWidget(m_tableDataView);
    dlgVBoxLayout->addWidget(dlgBtnFrame);
}

DialogDeleteImage::~DialogDeleteImage(){
    m_tableDataView->deleteLater();
    m_tableDataModel->deleteLater();
}

void DialogDeleteImage::setRootStorageList(const QStringList& mountStorages){
    for (int i = 0; i < mountStorages.size(); ++i){
        QFileInfo file(mountStorages[i]);
        QString imageFileName = file.fileName();
        imageFileName.remove(QRegularExpression("-id-\\d+"));
        QStandardItem* imageItem = new QStandardItem("   " + imageFileName);
        m_tableDataModel->setItem(i, 4, imageItem);
        m_rootStorages.push_back(imageFileName);
    }

    int tableHeight;
    int maxVisibleRows = 20;
    int rowsCount = 0;
    if (mountStorages.size() > maxVisibleRows){
        rowsCount = maxVisibleRows;
    }
    else {
        rowsCount = mountStorages.size();
    }

    tableHeight = m_tableDataView->horizontalHeader()->height()
                               + 2 +  rowsCount * m_tableDataView->rowHeight(0);
    m_tableDataView->setFixedHeight(tableHeight);
    m_tableDataView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void DialogDeleteImage::setDriveBusTypeList(const QStringList& driveBusTypes){

    for (int i = 0; i < driveBusTypes.size(); ++i){
        QStandardItem* numItem = new QStandardItem(QString::number(i + 1));
        QStandardItem* busItem = new QStandardItem(driveBusTypes[i]);

        numItem->setTextAlignment(Qt::AlignCenter);
        busItem->setTextAlignment(Qt::AlignCenter);

        m_tableDataModel->setItem(i, 0, numItem);
        m_tableDataModel->setItem(i, 1, busItem);
    }
}

void DialogDeleteImage::setDriveDevNameList(const QStringList& driveDevNames){

    for (int i = 0; i < driveDevNames.size(); ++i){
        QStandardItem* devItem = new QStandardItem(driveDevNames[i]);
        devItem->setTextAlignment(Qt::AlignCenter);
        m_tableDataModel->setItem(i, 2, devItem);
    }
}

void DialogDeleteImage::setDriveVirtSizeList(const QStringList& driveVirtSizes){
    for (int i = 0; i < driveVirtSizes.size(); ++i){
        QStandardItem* vSizeItem = new QStandardItem(driveVirtSizes[i]);
        vSizeItem->setTextAlignment(Qt::AlignCenter);
        m_tableDataModel->setItem(i, 3, vSizeItem);
    }
}

void DialogDeleteImage::setDriveChildren(const unsigned int& driveChildren){
    m_driveChildren = driveChildren;
}

void DialogDeleteImage::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    // *** Fix dialog width with drive table *** //
    this->setFixedHeight(this->height());
}

void DialogDeleteImage::rowSelect(const QItemSelection& selected){
    // *** Первое выделение включает чекбокс подтверждения *** //
    if (!m_rmConfirmation->isEnabled()){
        m_rmConfirmation->setEnabled(true);
    }

    if (!selected.indexes().isEmpty()) {
        int row = selected.indexes().first().row();

        QString confirmText;
        switch (m_driveChildren){
            case 0:
                confirmText = "Delete \"%1\"";
                confirmText = confirmText.arg(m_rootStorages[row]);
                break;
            case 1:
                confirmText = "Delete \"%1\" && child";
                confirmText = confirmText.arg(m_rootStorages[row]);
                break;
            default:
                confirmText = "Delete \"%1\" && %2 children";
                confirmText = confirmText.arg(m_rootStorages[row])
                                                        .arg(m_driveChildren);
        }
        m_rmConfirmation->setText(confirmText);
        m_rmConfirmation->setCheckState(Qt::Unchecked);
        m_selectedImageIndex = row;
    }
}

void DialogDeleteImage::deletePermissionChanged(Qt::CheckState state){
    QPushButton* okButton = m_dlgBtnBox->button(QDialogButtonBox::Ok);
    if (state == Qt::Checked){
        okButton->setEnabled(true);
    }
    else {
        okButton->setEnabled(false);
    }
}

void DialogDeleteImage::doDelete(){
    this->accept();
}

unsigned int DialogDeleteImage::getDeleteImagesIndex(){
    return m_selectedImageIndex;
}
// End dialogs.cpp
