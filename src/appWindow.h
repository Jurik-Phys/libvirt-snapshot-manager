// Begin appWindow.h

#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QApplication>
#include <QStyleFactory>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QBoxLayout>
#include <QToolButton>
#include <QStandardPaths>
#include "vmWidget.h"
#include "snapManager.h"
#include "snapTreeView.h"
#include "snapTreeModel.h"
#include "snapInfoWidget.h"
#include "vmDataCollector.h"
#include "treeItemDelegate.h"
#include "vmInfoWidget.h"
#include <QHeaderView>
#include "treeLinesStyle.h"

class QAppWindow : public QWidget {

    Q_OBJECT

    public:
        QAppWindow(QWidget* parent = nullptr);
        ~QAppWindow();

        void appExit();
        void closeEvent(QCloseEvent *event) override;
        void doSnapshot();
        void gotoSnapshot();
        void startVM();
        void openVM();
        void togglePauseVM();
        void deleteSnapshot();
        VMachine getActiveVm();
        bool checkExternalVmUtilities();

        // *** Manage VM images *** //
        bool addNewVmImages(const QStringList& newImageInfo);
        bool delVmImages(const unsigned int&);

    signals:
        void vmListProcessingStarted();
        void vmListProcessingCompleted();
        void startVmBegin();
        void startVmEnd();
        void selectVmChanged();
        void deleteActiveVm();

    public slots:
        void showErrorMessage(const QString& title, const QString& message);
        void onVmListReady(const QVector<VMachine> vmList);
        void onNewVmInfoReady(const VMachine&);

    private:
        const int m_appWindowWidth  = 945;
        const int m_appWindowHeight = 912;
        const int m_headFrameHeight = 90;
        const int m_infoFrameHeight = 328;

        const int m_btnHeight = 70;
        const int m_btnWidth = 1.2*m_btnHeight;
        const int m_vmIconSize = 48;
        const int m_vmWidgetHeight = m_vmIconSize + 4;
        int m_selectedVmIndex = -1;
        const QString m_libVirtConnectURI = "qemu:///system";

        VmDataCollector* m_vmDataCollector;
        QVBoxLayout*  m_vLColumnLayout;
        QVBoxLayout*  m_vRColumnLayout;
        QVBoxLayout*  m_vVmLayout;
        QVBoxLayout*  m_vSnapLayout;
        SnapTreeModel* m_snapTreeModel;
        SnapTreeModel* m_snapTreeLoadingModel;
        SnapTreeView*  m_snapTreeView;
        QVector<VMachine> m_vmList;
        VmInfoWidget* m_vmInfoWidget;
        SnapInfoWidget* m_snapInfoWidget;
        ChainNode     m_activeNode;
        QStringList   m_mountStorages;
        QStringList   m_rootVirtSizes;
        QStringList   m_driveBusTypes;
        QStringList   m_driveDevNames;
        QString       m_currentVmName;
        QString       m_currentVmUUID;
        QToolButton*  m_takeSnapBtn;
        QToolButton*  m_gotoBtn;
        QToolButton*  m_startBtn;
        QToolButton*  m_stopBtn;
        QToolButton*  m_deleteBtn;
        QToolButton*  m_pauseBtn;
        QToolButton*  m_openBtn;
        QMenu*  m_fullStopBtnMenu;
        QMenu*  m_onlyForceStopBtnMenu;
        QList<QAction*> m_defaulActionList;

        void setVmBtnFrame();
        void setVmFrame();
        void setVmInfoFrame();
        void setSnapBtnFrame();
        void setSnapFrame();
        void setSnapInfoFrame();

        void addVmToFrame();
        void addVmToFrame(const QVector<VMachine>&);
        void delVmFromFrame(const QVector<VMachine>&);
        void modVmIntoFrame(const QVector<VMachine>&);
        void updSnapTree();
        void btnManageGoDel();
        void btnManageStart();
        void btnManagePause();
        void btnManageStop();
        void btnManageTake();
        void snapTreeViewManage();
        void vmInfoWidgetManage();
        void snapInfoWidgetManage();

        void onTreeItemClicked(const QModelIndex& index);
        void resizeEvent(QResizeEvent *event) override;

        QVector<VMachine> getToAddVmList(const QVector<VMachine>& appList,
                                         const QVector<VMachine>& inList);
        QVector<VMachine> getToDelVmList(const QVector<VMachine>& appList,
                                         const QVector<VMachine>& inList);
        QVector<VMachine> getToModVmList(const QVector<VMachine>& appList,
                                         const QVector<VMachine>& inList);
        int getInsertWidgetIndex(const QVector<VMachine>& appList,
                                                          const VMachine& inVm);
        int getDeleteWidgetIndex(const QVector<VMachine>& appList,
                                                          const VMachine& inVm);
        int getModifyWidgetIndex(const QVector<VMachine>& appList,
                                                          const VMachine& inVm);
        void menuRebootVM();
        void menuShutDownVM();
        void menuForceRebootVM();
        void menuForceShutdownVM();
        void menuStopBtnSelect();

        void viewOnlyMode();

        bool checkVmUtilityAvailable(const QString&);
        bool checkVmUtilityExecutable(const QString&);
        bool checkLocalHypervisorConnection();
        bool isLibvirtPolkitEnabled();

        int getSnapImagesId(const QString& imageFullName);
};

#endif
// End appWindow.h
