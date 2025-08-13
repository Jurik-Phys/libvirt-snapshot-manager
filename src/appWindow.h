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
#include "vmWidget.h"
#include "snapManager.h"
#include "snapTreeView.h"
#include "snapTreeModel.h"
#include "vmDataCollector.h"
#include "treeItemDelegate.h"
#include <QHeaderView>
#include "treeLinesStyle.h"

class QAppWindow : public QWidget {

    Q_OBJECT

    public:
        QAppWindow(QWidget* parent = nullptr);
        ~QAppWindow();

        void appExit();
        void doSnapshot();
        void gotoSnapshot();
        void startVM();
        void openVM();
        void togglePauseVM();
        void deleteSnapshot();
        VMachine getActiveVm();

    signals:
        void vmListProcessingStarted();
        void vmListProcessingCompleted();
        void selectVmChanged();

    public slots:
        void showErrorMessage(const QString& title, const QString& message);
        void onVmListReady(const QVector<VMachine> vmList);

    private:
        const int m_appWindowWidth  = 945;
        const int m_appWindowHeight = 750;
        const int m_headFrameHeight = 90;

        const int m_btnHeight = 64;
        const int m_btnWidth = 1.2*m_btnHeight;
        const int m_vmIconSize = 48;
        int m_selectedVmIndex = -1;

        QVBoxLayout*  m_vLColumnLayout;
        QVBoxLayout*  m_vRColumnLayout;
        QVBoxLayout*  m_vVmLayout;
        QVBoxLayout*  m_vSnapLayout;
        SnapTreeModel* m_snapTreeModel;
        SnapTreeModel* m_snapTreeLoadingModel;
        SnapTreeView*  m_snapTreeView;
        QVector<VMachine> m_vmList;
        ChainNode     m_activeNode;
        QStringList   m_mountStorages;
        QString       m_currentVmName;
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
        void setSnapBtnFrame();
        void setSnapFrame();

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
};

#endif
// End appWindow.h
