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
#include "snapManager.h"
#include "snapTreeModel.h"
#include "vmWidget.h"
#include "vmDataCollector.h"
#include <QTreeView>
#include <QHeaderView>

class QAppWindow : public QWidget {

    Q_OBJECT

    public:
        QAppWindow(QWidget* parent = nullptr);
        ~QAppWindow();

        void appExit();
        void takeSnap();
        VMachine getActiveVm();


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
        QTreeView*     m_snapTreeView;
        QVector<VMachine> m_vmList;
        ChainNode     m_activeNode;

        void setVmBtnFrame();
        void setVmFrame();
        void setSnapBtnFrame();
        void setSnapFrame();

        void addVmToFrame();
        void updSnapTree();

        void onTreeItemClicked(const QModelIndex& index);
        void resizeEvent(QResizeEvent *event) override;

};

#endif
// End appWindow.h
