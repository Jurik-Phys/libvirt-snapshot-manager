// Begin snapTreeView.cpp

#include "snapTreeView.h"
#include "snapTreeModel.h"

SnapTreeView::SnapTreeView(QWidget* parent) : QTreeView(parent){
    setContextMenuPolicy(Qt::DefaultContextMenu);
}

void SnapTreeView::contextMenuEvent(QContextMenuEvent* event){
    contextIndex = indexAt(event->pos());

    if (!contextIndex.isValid()){
        return;
    }

    emit clicked(contextIndex);

    SnapTreeModel* model = qobject_cast<SnapTreeModel*>(this->model());
    ChainNode node = model->getChainNodeByIndex(contextIndex);

    QMenu menu(this);
    if (node.imagesType == "active"){
        QAction* runVmAction  = menu.addAction("Start VM at this state");
        QAction* takeAction   = menu.addAction("Take snapshot …");

        QObject::connect(runVmAction, &QAction::triggered,
                                            this, &SnapTreeView::onActionRunVm);
        QObject::connect(takeAction, &QAction::triggered,
                                       this, &SnapTreeView::onActionDoSnapshot);
    } else {
        QAction* gotoAction   = menu.addAction("Go to snapshot");
        QAction* deleteAction = menu.addAction("Delete snapshot …");

        QObject::connect(deleteAction, &QAction::triggered,
                                           this, &SnapTreeView::onActionDelete);
        QObject::connect(gotoAction, &QAction::triggered,
                                             this, &SnapTreeView::onActionGoto);
    }

    menu.exec(event->globalPos());
}

void SnapTreeView::onActionRunVm(){
    emit startVM();
}

void SnapTreeView::onActionDoSnapshot(){
    emit doSnapshot();
}

void SnapTreeView::onActionDelete(){
    emit deleteSnapshot();
}

void SnapTreeView::onActionGoto(){
    emit gotoSnapshot();
}

// End snapTreeView.cpp
