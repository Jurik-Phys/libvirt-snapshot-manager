// Begin snapTreeModel.cpp

#include "snapTreeModel.h"

SnapTreeModel::SnapTreeModel(const QVector<ChainNode>& nodes, QObject* parent) :
                                                    QAbstractItemModel(parent){
    m_nodes = nodes;
}

SnapTreeModel::SnapTreeModel(QObject* parent) : QAbstractItemModel(parent){
}


SnapTreeModel::~SnapTreeModel(){
}

// Если в {row, column} есть узел, то возвращаем его индекс
QModelIndex SnapTreeModel::index(int row, int column,
                                              const QModelIndex& parent) const {
    int parentId = -1;
    if (parent.isValid()){
        parentId = parent.internalId();
    }

    int childId = findChildIdByRow(parentId, row);
    if (childId == -1) {
        return QModelIndex();
    }
    else {
        return createIndex(row, column, childId);
    }
}

int SnapTreeModel::findChildIdByRow(int parentId, int row) const {

   /*
    * Потомков у одного узла может быть много, поэтому в QAbstractItemModel
    * узел представлен, как линейный спискок со строками (row):
    *
    *   Родитель (id = 1)
    *   |- Child A (row = 0)
    *   |- Child B (row = 1)
    *   |- Child C (row = 2)
    *
    *   {
    *       {id, parentId,    name}
    *       { 1,       -1,  "Root"},
    *       { 2,        1,     "A"},
    *       { 3,        1,     "B"},
    *       { 4,        1,     "C"},
    *       { 5,        3,   "B.1"},
    *   }
    *
    *   findChildIdByRow(int parentId, int row);
    *
    *   findChildIdByRow(1, 0) => 2 ("A")
    *   findChildIdByRow(1, 1) => 3 ("B")
    *   findChildIdByRow(1, 2) => 4 ("C")
    *   findChildIdByRow(3, 0) => 5 ("B.1")
    */

    int count = -1;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].parentId == parentId) {
            ++count;
            if (count == row)
                return m_nodes[i].id;
        }
    }
    return -1;
}

QModelIndex SnapTreeModel::parent(const QModelIndex& index) const {
    if (!index.isValid()) {
        return QModelIndex();
    }
    else {
        int id = index.internalId();
        int parentId = findNodeById(id).parentId;
        if (parentId == -1) {
            return QModelIndex();
        }
        else {
           /* parentId'а родителя (то есть "дедушки") необходим, чтобы найти
            * в какой строке (row) родитель находится среди своих "братьев".
            * Это необходимо для createIndex(int row, int column, ...),
            * который требует строку (row) и колонку (column) и ID.
            *
            * Пример:
            *
            *   - Root (id = 1)
            *      |-- A (id = 2, parentId = 1)
            *      |   |-- A1 (id = 4, parentId = 2)
            *      |---B (id = 3, parentId = 1)
            *
            *    parent(A1):
            *      index.internalId() => 4
            *      findNodeById(4).parentId => 2 (это A)
            *      findNodeById(2).parentId => 1 (это Root)
            *      rowOfChild(2, 1) → 0 (A — первый ребёнок Root)
            *      createIndex(0, 0, 2) => вернёт индекс на A
            *
            */

            int grandParentId = findNodeById(parentId).parentId;
            int row = rowOfChild(parentId, grandParentId);
            return createIndex(row, 0, parentId);
        }
    }
}

const ChainNode& SnapTreeModel::findNodeById(int id) const {
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].id == id)
            return m_nodes[i];
    }
    static ChainNode fakeNode{-1, -1, QStringLiteral("<invalid>")};
    return fakeNode;
}

int SnapTreeModel::rowOfChild(int childId, int parentId) const {
    int row = 0;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].parentId == parentId) {
            if (m_nodes[i].id == childId)
                return row;
            ++row;
        }
    }
    return -1;
}

int SnapTreeModel::rowCount(const QModelIndex& parent) const {
    int parentId;
    if (parent.isValid()){
       parentId = parent.internalId();
    } else {
       parentId = -1;
    }

    int count = 0;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].parentId == parentId) {
            ++count;
        }
    }

    return count;
}

int SnapTreeModel::columnCount(const QModelIndex&) const {
    return 1;
}

QVariant SnapTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole){
        return QVariant();
    }
    else {
        int id = index.internalId();
        const auto& node = findNodeById(id);
        return node.name;
    }
}

void SnapTreeModel::setSnapData(const QVector<ChainNode>& nodes) {
    m_nodes = nodes;
}

ChainNode SnapTreeModel::getChainNodeByIndex(const QModelIndex& index){
    ChainNode res;
    int nodeId = index.internalId();
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].id == nodeId) {
            return m_nodes[i];
        }
    }
    static ChainNode dummy{-1, -1, QStringLiteral("<invalid>")};
    return dummy;
}

bool SnapTreeModel::removeRows(int row, int count, const QModelIndex& parent){
    bool res;


    return res;
}

// End snapTreeModel.cpp
