// Begin snapTreeView.h
#ifndef SNAPTREEVIEW_H
#define SNAPTREEVIEW_H

#include <QMenu>
#include <QTreeView>
#include <QContextMenuEvent>

class SnapTreeView : public QTreeView {

    Q_OBJECT

    public:
        SnapTreeView(QWidget* parent = nullptr);

    signals:
        void doSnapshot();
        void deleteSnapshot();
        void gotoSnapshot();
        void startVM();

    protected:
        void contextMenuEvent(QContextMenuEvent* event) override;

    private slots:
        void onActionGoto();
        void onActionDelete();
        void onActionDoSnapshot();
        void onActionRunVm();

    private:
        QModelIndex contextIndex;
};

#endif
// End snapTreeView.h
