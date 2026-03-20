// Begin dialogDeleteImage.cpp

#include "dialogDeleteImage.h"

#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QFormLayout>
#include <QFileDialog>

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

// End dialogDeleteImage.cpp
