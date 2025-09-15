// Begin snapTreeView.cpp

#include "snapTreeView.h"
#include "snapTreeModel.h"

SnapTreeView::SnapTreeView(QWidget* parent) : QTreeView(parent){
    setContextMenuPolicy(Qt::DefaultContextMenu);
}

bool SnapTreeView::isSelectItem(){
    bool res = false;

    QModelIndexList selected = this->selectionModel()->selectedIndexes();

    if (selected.size() > 0){
        res = true;
    }

    return res;
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
        QIcon runVmActionIcon = QIcon(":/btn-vm-start.svg");
        runVmAction->setIcon(runVmActionIcon);

        QAction* takeAction   = menu.addAction("Take snapshot…");
        QIcon takeActionIcon = QIcon(":/btn-take-snapshot.svg");
        takeAction->setIcon(takeActionIcon);

        QObject::connect(runVmAction, &QAction::triggered,
                                            this, &SnapTreeView::onActionRunVm);
        QObject::connect(takeAction, &QAction::triggered,
                                       this, &SnapTreeView::onActionDoSnapshot);
    } else {
        QAction* gotoAction   = menu.addAction("Go to snapshot");
        QIcon gotoActionIcon = QIcon(":/btn-goto-snapshot-16.png");
        gotoAction->setIcon(gotoActionIcon);

        QAction* deleteAction = menu.addAction("Delete snapshot…");
        QIcon deleteActionIcon = QIcon(":/btn-delete-snapshot-16.png");
        deleteAction->setIcon(deleteActionIcon);

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
