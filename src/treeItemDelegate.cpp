// Begin treeItemDelegate.cpp

#include "treeItemDelegate.h"

TreeItemDelegate::TreeItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent){
}

void TreeItemDelegate::paint(QPainter* painter,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // Сохранение изначального текста
    QString text = opt.text;

    // *** Получение модели *** //
    const SnapTreeModel* model = qobject_cast<const SnapTreeModel*>
                                                                (index.model());
    if (!model)
        return;
    ChainNode node = model->getChainNodeByIndex(index);

    // *** Модификация названий для учёта их типа *** //
    if (node.id != -1){
        if (node.imagesType == "work"){
            text = ">> " + text + " …";
        }
        else {
            if (node.imagesType == "snap"){
                text = "❄ " + text + " ❄";
            } else {
                if (node.imagesType == "active" && node.parentId == -1){
                    text = "★★★ You Are Here! ★★★ (" + text + ")";
                }
                else {
                    if (!text.contains("Loading snapshot chain information")){
                        text = "★★★ You Are Here! ★★★ (" + text + ")";
                    }
                }
            }
        }
    }

    // *** Применение измений названий *** //
    opt.text = text;

    const QWidget *widget = option.widget;
    QStyle *style = widget->style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

}

QSize TreeItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const {

    const SnapTreeModel* model = qobject_cast<const SnapTreeModel*>
                                                                (index.model());
    ChainNode node = model->getChainNodeByIndex(index);

    QSize size = QStyledItemDelegate::sizeHint(option, index);

    if (node.imagesType == "work" || node.imagesType == "snap"){
        size.rwidth() += 75;
    }
    else {
        size.rwidth() += 200;
    }
    return size;
}

// End treeItemDelegate.cpp
