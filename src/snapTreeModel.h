// Begin snapTreeModel.h

#ifndef SNAPTREEMODEL_H
#define SNAPTREEMODEL_H

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
 *      SnapNode root = { 1, -1,  "Root"};
 *      SnapNode a    = { 2,  1,     "A"};
 *      SnapNode b    = { 3,  1,     "B"};
 *      SnapNode c    = { 4,  1,     "C"};
 *      SnapNode b1   = { 5,  3,   "B.1"};
 *      SnapNode b2   = { 6,  3,   "B.2"};
 *      SnapNode a1   = { 7,  2,   "A.1"};
 *      SnapNode a2   = { 8,  2,   "A.2"};
 */

#include <QString>
#include <QAbstractItemModel>

struct SnapNode {
    int id;
    int parentId;
    QString name;
    QString info;
    QString filePath;
};

class SnapTreeModel : public QAbstractItemModel {

    Q_OBJECT

    public:
        SnapTreeModel(const QVector<SnapNode>&, QObject* parent = nullptr);
        SnapTreeModel(QObject* parent = nullptr);
        ~SnapTreeModel();

        QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& = QModelIndex()) const override;
        QVariant data(const QModelIndex& index,
                                     int role = Qt::DisplayRole) const override;
        void setSnapData(const QVector<SnapNode>&);

        SnapNode getSnapNodeByIndex(const QModelIndex& index);

    private:
        QVector<SnapNode> m_nodes;

        int findChildIdByRow(int parentId, int row) const;
        const SnapNode& findNodeById(int id) const;
        int rowOfChild(int childId, int parentId) const;

};

#endif

// End snapTreeModel.h
