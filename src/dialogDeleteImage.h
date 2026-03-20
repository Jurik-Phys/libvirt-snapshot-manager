// Begin dialogDeleteImage.h

#ifndef DIALOGDELETEIMAGE_H
#define DIALOGDELETEIMAGE_H

#include <QDialog>
#include <QCheckBox>
#include <QTableView>
#include <QDialogButtonBox>
#include <QStandardItemModel>

class DialogDeleteImage : public QDialog {

    Q_OBJECT

    public:
        DialogDeleteImage(QWidget* parent = nullptr);
        ~DialogDeleteImage();

        void setRootStorageList(const QStringList&);
        void setDriveBusTypeList(const QStringList&);
        void setDriveDevNameList(const QStringList&);
        void setDriveVirtSizeList(const QStringList&);
        void setDriveChildren(const unsigned int&);
        unsigned int getDeleteImagesIndex();

    private:
        const int m_dlgTitleFrameHeight = 55;
        QDialogButtonBox* m_dlgBtnBox;
        QCheckBox*        m_rmConfirmation;
        void showEvent(QShowEvent*);
        void rowSelect(const QItemSelection&);
        QStringList m_rootStorages;
        QStringList m_driveBusTypes;
        QStringList m_driveDevNames;
        QStringList m_driveVirtSizes;

        unsigned int m_driveChildren;
        unsigned int m_selectedImageIndex;

        QStandardItemModel* m_tableDataModel;
        QTableView*         m_tableDataView;
        void deletePermissionChanged(Qt::CheckState);
        void doDelete();
};

#endif
// End dialogDeleteImage.h
