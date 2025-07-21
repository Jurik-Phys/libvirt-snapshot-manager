// Begin snapTreeModel.h

#ifndef SNAPTREEMODEL_H
#define SNAPTREEMODEL_H

#include "vmDataStructs.h"
/*
 * Модель данных реализуется посредством следующих пяти функций:
 *
 *   QModelIndex index(int row, int column, const QModelIndex &parent);
 *   QModelIndex parent(const QModelIndex &child);
 *   int rowCount(const QModelIndex &parent = QModelIndex());
 *   int columnCount(const QModelIndex &parent = QModelIndex())
 *   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole);
 *
 *   // Data example {id, parentId, name}
 *      ChainNode root = { 1, -1,  "Root"};
 *      ChainNode a    = { 2,  1,     "A"};
 *      ChainNode b    = { 3,  1,     "B"};
 *      ChainNode c    = { 4,  1,     "C"};
 *      ChainNode b1   = { 5,  3,   "B.1"};
 *      ChainNode b2   = { 6,  3,   "B.2"};
 *      ChainNode a1   = { 7,  2,   "A.1"};
 *      ChainNode a2   = { 8,  2,   "A.2"};
 */

#include <QString>
#include <QAbstractItemModel>

class SnapTreeModel : public QAbstractItemModel {

    Q_OBJECT

    public:
        SnapTreeModel(const QVector<ChainNode>&, QObject* parent = nullptr);
        SnapTreeModel(QObject* parent = nullptr);
        ~SnapTreeModel();

        QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& = QModelIndex()) const override;
        QVariant data(const QModelIndex& index,
                                     int role = Qt::DisplayRole) const override;
        void setSnapData(const QVector<ChainNode>&);

        ChainNode getChainNodeByIndex(const QModelIndex& index);

    private:
        QVector<ChainNode> m_nodes;

        int findChildIdByRow(int parentId, int row) const;
        const ChainNode& findNodeById(int id) const;
        int rowOfChild(int childId, int parentId) const;

};

#endif

// End snapTreeModel.h
