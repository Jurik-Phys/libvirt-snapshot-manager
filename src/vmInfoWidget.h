// Begin vmInfoWidget.h
#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QProcess>
#include <QEventLoop>
#include <QKeyEvent>
#include <QEvent>
#include <QDomDocument>
#include <QToolButton>
#include <QMenu>
#include <QFile>
#include <QLabel>
#include <QTimer>
#include "vmDataStructs.h"
#include "dialogs.h"

class VmInfoWidget : public QFrame {

    Q_OBJECT

    public:
        VmInfoWidget(QWidget* parent = nullptr);
        ~VmInfoWidget();

        void setData(const VMachine&);
        VMachine getData();
        void setStorageList(const QStringList&);
        void setDriveBusTypeList(const QStringList&);
        void setDriveDevNameList(const QStringList&);
        void setDriveVirtSizeList(const QStringList&);
        void setDriveChildren(const unsigned int&);

        void setName(const QString&);
        void setTitle(const QString&);
        void setDescription(const QString&);
        void setCpu(const QString&);
        void setRam(const QString&);
        void setOsName(const QString&);
        void setReadOnly(bool);
        void clearData();

    signals:
        void textChangedBegin();
        void textChangedEnd();
        void writeVmTitleRequested(const QString& vm_uuid,
                                                        const QString& vmTitle);
        void writeVmDescriptionRequested(const QString& vm_uuid,
                                                  const QString& vmDescription);
        void addNewVmImagesRequested(const QStringList& newImageInfo);
        void delVmImagesRequested(const unsigned int&);

        void requestVmOsInfoUpdate();
        void requestVmOsXmlInfoClear();
        void requestVmOsXmlInfoUpdate(const OsInfo&);

    public slots:
        void onRequestVmOsInfoUpdate();
        void onRequestVmOsXmlInfoClear();
        void onRequestVmOsXmlInfoUpdate(const OsInfo&);

    private:
        QVBoxLayout* m_vScrollLayout;
        void fixScrollBar(QScrollArea*);
        void fixScrollBar(QTextEdit*);

        QVector<QWidget*> m_colAWidgets;
        QVector<QWidget*> m_colBWidgets;
        QString humanMemory(const QString&);
        QString cutLongOsName(const QString&, const int&);

        QToolButton* m_editImagesBtn;
        QToolButton* m_editRamSizeBtn;
        QToolButton* m_editCpuTopologyBtn;
        QToolButton* m_editOsBtn;
        void manageDeleteItem();

        QString m_uuid;
        QStringList m_mountStorages;
        QStringList m_driveBusTypes;
        QStringList m_driveDevNames;
        QStringList m_driveVirtSizes;
        unsigned int m_driveChildren;

        void writeVmTitle();
        void writeVmDescription();
        void restartSaveTitleTimer();
        void restartSaveDescriptionTimer();
        bool eventFilter(QObject* obj, QEvent* event) override;

        QTimer* m_saveTitleTimer;
        QTimer* m_saveDescriptionTimer;

        int m_titleChangedCounter = 0;
        int m_descriptionChangedCounter = 0;
        int calcOptimalFontSize(const QStringList&);

        // *** Guest OS menu *** //
        void manageGuestOS();

        // *** Mounted drives menu ***//
        void addNewVmImages();
        void delVmImages();

        void setEditBtnStyle(QToolButton*);

        QString getCurrentGuestOS();
};

#endif
// End vmInfoWidget.h
