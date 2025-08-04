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
        void deleteSnapshot();
        VMachine getActiveVm();

    public slots:
        void showErrorMessage(const QString& title, const QString& message);

    private:
        const int m_appWindowWidth  = 945;
        const int m_appWindowHeight = 750;
        const int m_headFrameHeight = 94;

        const int m_btnSize1 = 64;
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
        QToolButton*  m_deleteBtn;

        void setVmBtnFrame();
        void setVmFrame();
        void setSnapBtnFrame();
        void setSnapFrame();

        void addVmToFrame();
        void updSnapTree();
        void takeSnapBtnManage();
        void gotoAndDelBtnManage();
        void startBtnManage();

        void onTreeItemClicked(const QModelIndex& index);
        void resizeEvent(QResizeEvent *event) override;
};

#endif
// End appWindow.h
