// Begin dialogs.cpp

#include "dialogs.h"

DialogAddNewImage::DialogAddNewImage(QWidget* parent) : QDialog(parent){
    setWindowTitle("New VM Image (qcow2)");
    setModal(true);
    setMinimumWidth(m_leftWidth*4.8);

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
                 setPlaceholderText("Enter image filename (e.g. debian.qcow2)");
    hFileNameLayout->addWidget(fileNameLabel);
    hFileNameLayout->addWidget(m_fileNameEdit);
    vDialogLayout->addLayout(hFileNameLayout);

    // *** Capacity & Error output *** //
    QHBoxLayout* hCapacityLayout = new QHBoxLayout();
    QLabel* capacityLabel = new QLabel("Capacity:");
    capacityLabel->setFixedWidth(m_leftWidth);
    m_capacitySpinBox = new QDoubleSpinBox();
    m_capacitySpinBox->setRange(0.0, 99999.9);
    m_capacitySpinBox->setValue(256.0);
    m_capacitySpinBox->setSingleStep(0.1);
    m_capacitySpinBox->setDecimals(1);
    m_capacitySpinBox->setSuffix(" GiB");
    m_capacitySpinBox->setFixedWidth(qRound(m_leftWidth * 1.15));
    hCapacityLayout->addWidget(capacityLabel);
    hCapacityLayout->addWidget(m_capacitySpinBox);
    m_errorOut = new QLabel();
    m_errorOut->setStyleSheet("font-weight: bold; color: red;");
    hCapacityLayout->addStretch();
    hCapacityLayout->addWidget(m_errorOut);
    vDialogLayout->addLayout(hCapacityLayout);

    // *** Dialog buttons *** //
    QDialogButtonBox* dlgBtnBox = new QDialogButtonBox(
                                                QDialogButtonBox::Ok |
                                                QDialogButtonBox::Cancel, this);
    QObject::connect(dlgBtnBox, &QDialogButtonBox::accepted,
                                   this, &DialogAddNewImage::checkSelectedPath);
    QObject::connect(dlgBtnBox, &QDialogButtonBox::rejected,
                                              this, &DialogAddNewImage::reject);
    vDialogLayout->addWidget(dlgBtnBox);

    // *** Apply vDialogLayout *** //
    setLayout(vDialogLayout);
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
        m_errorOut->setText("Directory write access denied.");
        return;
    }

    // *** Проверка корректности имени файла *** //
    QString fName = m_fileNameEdit->text().trimmed();
    if (!isValidFileName(fName)){
        if (fName.size() < 1){
            m_errorOut->setText("No image filename.");
        }
        else {
            m_errorOut->setText("Invalid filename.");
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
        m_errorOut->setText("The file already exists.");
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
    if (forbidden.match(fName).hasMatch())
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

// End dialogs.cpp
