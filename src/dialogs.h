// Begin dialogs.h

#ifndef DIALOGS_H
#define DIALOGS_H

#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialog>
#include <QFileDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QHeaderView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QStandardItemModel>
#include <QTableView>
#include <QDoubleSpinBox>
#include <QTemporaryFile>
#include <QToolButton>
#include <QStringListModel>
#include "osInfoProvider.h"

class DialogAddNewImage : public QDialog {

    Q_OBJECT

    public:
        DialogAddNewImage(QWidget* parent = nullptr);
        ~DialogAddNewImage();

        QString getImageFullName();
        QString getImageSize();
        QString getImageBusType();
        long int capacity;
        void setVmName(const QString& vmName);

    private:
        void checkSelectedPath();
        void selectDir();
        bool isValidFileName(const QString& fName);
        bool isValidDirPath(const QString& dir);
        QLineEdit*      m_fileNameEdit;
        QLineEdit*      m_dirPathEdit;
        QDoubleSpinBox* m_capacitySpinBox;
        QLabel*         m_errorOut;
        QComboBox*      m_controllerBox;
        int m_leftWidth = 100;
};

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

class DialogManageGuestOS : public QDialog {

    Q_OBJECT

    public:
        DialogManageGuestOS(const QString& guestOS, QWidget* parent = nullptr);
        ~DialogManageGuestOS();

        void setInfo();
        void printLatesLibOsInfoVersion();

    signals:
        void requestVmOsInfoUpdate();
        void requestVmOsXmlInfoClear();
        void requestVmOsXmlInfoUpdate(const OsInfo&);

    public slots:
        void doUpdateLibOsInfo();

    private:
        QComboBox* m_osEditor;
        QString m_inGuestOS;
        QString formatVersionString(const unsigned int&);
        void updateLocalLibOsInfo();
        OsInfoProvider* m_osInfoProvider;
        const int m_dlgTitleFrameHeight = 55;
        QLabel* m_updateInfoError;
        QLabel* m_lastLibOsInfoVersion;
        QLabel* m_libOsInfoVersion;
        QToolButton* m_updateLocalLibOsInfo;
        QDialogButtonBox* m_dlgBtnBox;
        int m_leftWidth = 175;
        int m_lblIndent = 25;

        QFormLayout* m_infoFrameLayout;
        QHBoxLayout* m_remoteLibOsInfoHLayout;
        QHBoxLayout* m_lastLibOsInfoHLayout;

        QString m_upToDate = "(no update required)";

        QStringListModel* m_osNameListModel;
};

#endif
// End dialogs.h
