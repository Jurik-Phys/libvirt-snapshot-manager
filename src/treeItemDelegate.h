// Begin treeItemDelegate.h

#ifndef TREEITEMDELEGATE_H
#define TREEITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QApplication>
#include "snapTreeModel.h"
#include "vmDataStructs.h"

class TreeItemDelegate : public QStyledItemDelegate {

    Q_OBJECT

    public:
        TreeItemDelegate(QObject* parent = nullptr);

        void paint(QPainter* painter,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

        QSize sizeHint(const QStyleOptionViewItem& option,
                       const QModelIndex& index) const override;
};

#endif
// End treeItemDelegate.h
