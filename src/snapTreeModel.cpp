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

ChainNode SnapTreeModel::getChainNodeByIndex(const QModelIndex& index) const {
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

    // Узел родителя
    ChainNode parentNode = getChainNodeByIndex(parent);

    QModelIndex grandParent = parent.parent();
    // Уведомление QTreeView о том, что ожидается удаление строк(и)
    // beginRemoveRows(const QModelIndex &parent, int first, int last)
    beginRemoveRows(parent, row, row + count - 1);

    // Удаление данных из вектора узлов
    for (int i = 0; i < count; ++i) {
        // Надо найти потомка и удалить его из m_nodes;
        // Признак того, что потомок
        for (int i = 0; i < m_nodes.size(); ++i){

            int delNodeId = m_nodes[i].id;
            int delNodeParentId = m_nodes[i].parentId;

            int childId = findChildIdByRow(parentNode.id, row);
            if ( childId == m_nodes[i].id ){
                // При удалении потомка, у родителя в векторе
                // "childrenImagesFullNames" необходимо удалить
                // списки дисков удаляемого потомка
                QStringList childImagesList = m_nodes[i].imagesFullNames;
                for (int j = 0; j < m_nodes.size(); ++j){
                    if (parentNode.id == m_nodes[j].id){
                        m_nodes[j].childrenImagesFullNames
                                                    .removeOne(childImagesList);
                    }
                }
                m_nodes.removeAt(i);

                // Индекс в m_nodes родителя, удаляемого узла;
                int parentIdx = -1;
                for (int n = 0; n < m_nodes.size(); ++n){
                    if ( parentNode.id == m_nodes[n].id){
                        parentIdx = n;
                    }
                }

                // После удаления узла, весь список узлов разделяется
                // на а) верхнюю неизменную часть, б) удаляемый узел
                // и в) нижнюю часть, в которой необходимо сделать
                // преобразования id и parentId
                for (int k = i; k < m_nodes.size(); ++k){
                    // 1. Для всех узлов необходимо уменьшить id единицу
                    m_nodes[k].id -=1;

                    // 2. Если кто-то имел родителем удаляемый узел т.е.,
                    //    у него в поле parentId был записан id удаляемого узла,
                    //    то подключем его к родителю удаляемого узла т.е.,
                    //    вносим изменения в его поле parentId
                    if (m_nodes[k].parentId == delNodeId){
                        // *** Изменение parentId потомка удаляемого узла *** /
                        m_nodes[k].parentId = delNodeParentId;

                        if (parentIdx != -1){
                            // m_nodes[k] - потомок удаляемого узла, его данные
                            // в виде imagesFullNames необходимо добавить в
                            // вектор "childrenImagesFullNames" нового родителя
                            m_nodes[parentIdx].childrenImagesFullNames
                                         .push_back(m_nodes[k].imagesFullNames);

                            // У потомка удаляемого узла необходимо изменить
                            // поле "backFullNames" т.к.,  теперь он будет
                            // потомком другого родителя
                            m_nodes[k].backFullNames = m_nodes[parentIdx]
                                                               .imagesFullNames;
                        }
                        else{
                            // Нового родителя нет т.к., узел корневой,
                            // добавлять информацию о потомках некому,
                            // зато необходимо очистить backFullNames;
                            for (int j = 0; j < m_nodes[k].backFullNames.size();
                                                                           ++j){
                                m_nodes[k].backFullNames[j] = "None";
                            }
                        }
                    }
                    // 3. Если родитель не удаляемый узел, то возможно две
                    //    ситуации:
                    //    - родитель в блоке с изменившимися id (низ списка)
                    //    - родитель в неизменном блоке (верх списка)
                    //    В первом случае необходимо parentId уменьшить на "1",
                    //    во втором случае ничего делать не требуется
                    else {
                        if (m_nodes[k].parentId > delNodeId){
                            m_nodes[k].parentId -=1;
                        }
                    }
                }
            }
        }
    }

    // Проверка родителя, удаляемого узла на изменение типа
    // Если потомков нет (получен индекс "-1"),
    // то узел из типа "snap" переходит в "work"
    // row = 0 соответствует первому потомку
    int isWork = findChildIdByRow(parentNode.id, 0);
    if (isWork == -1){ // Потомков нет
        for (int i = 0; i < m_nodes.size(); ++i){
            if (m_nodes[i].id == parentNode.id){
                m_nodes[i].imagesType = "work";
            }
        }
    }

    endRemoveRows();

    return true;
}

bool SnapTreeModel::insertRows(int row, int count, const QModelIndex& index){

    // *** Поиск интекса idx узла в m_nodes с imagesType == active *** //
    int activeIndex = -1;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].imagesType == "active") {
            activeIndex = i;
        }
    }

    beginInsertRows(index, row, row + count - 1);
        VmDataCollector vmDataCollect;
        // *** Изменение параметров активного узла в данный момент *** //
        m_nodes[activeIndex].imagesType = "snap";
        m_nodes[activeIndex].childrenImagesFullNames
                                              .push_back(m_snapImagesFullNames);
        // *** Формирование узла, его параметров *** //
        ChainNode newNode;
        // id
        newNode.id = m_nodes[m_nodes.size() - 1].id + 1;
        // parentId
        newNode.parentId = m_nodes[activeIndex].id;
        // uuid
        newNode.uuid = vmDataCollect.getNodeUuid(m_snapImagesFullNames.last());
        // imagesType
        newNode.imagesType = "active";
        // imagesFullNames
        newNode.imagesFullNames = m_snapImagesFullNames;
        // backFullNames
        for (int i = 0; i < m_snapImagesFullNames.size(); ++i){
            newNode.backFullNames
                            .push_back(m_nodes[activeIndex].imagesFullNames[i]);
        }
        // name
        newNode.name = vmDataCollect.getNodeName(newNode.uuid);
        // *** Добавление нового узла в данные модели *** //
        m_nodes.push_back(newNode);
    endInsertRows();

    return true;
}

bool SnapTreeModel::insertRowAt(int row, const QModelIndex& parentIndex){

    ChainNode node = getChainNodeByIndex(parentIndex);

    // *** Поиск интекса idx узла в m_nodes *** //
    int parentNodeIdx = -1;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].id == node.id) {
            parentNodeIdx = i;
        }
    }

    // *** Поиск интекса idx узла в m_nodes с imagesType == active *** //
    int activeIndex = -1;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].imagesType == "active") {
            activeIndex = i;
        }
    }

    beginInsertRows(parentIndex, row, row - 1);
        VmDataCollector vmDataCollect;
        // *** Изменение параметров активного узла в данный момент *** //
        m_nodes[activeIndex].imagesType = "work";

        // *** Изменение параметров активного узла в данный момент *** //
        m_nodes[parentNodeIdx].childrenImagesFullNames
                                              .push_back(m_snapImagesFullNames);
        // *** Формирование узла, его параметров *** //
        ChainNode newNode;
        // id
        newNode.id = m_nodes[m_nodes.size() - 1].id + 1;
        // parentId
        newNode.parentId = m_nodes[parentNodeIdx].id;
        // uuid
        newNode.uuid = vmDataCollect.getNodeUuid(m_snapImagesFullNames.last());
        // imagesType
        newNode.imagesType = "active";
        // imagesFullNames
        newNode.imagesFullNames = m_snapImagesFullNames;
        // backFullNames
        for (int i = 0; i < m_snapImagesFullNames.size(); ++i){
            newNode.backFullNames
                          .push_back(m_nodes[parentNodeIdx].imagesFullNames[i]);
        }
        // name
        newNode.name = vmDataCollect.getNodeName(newNode.uuid);
        // *** Добавление нового узла в данные модели *** //
        m_nodes.push_back(newNode);
    endInsertRows();

    return true;
}

void SnapTreeModel::setSnapImagesFullName(QStringList imagesFullNames){
    m_snapImagesFullNames = imagesFullNames;
}

void SnapTreeModel::addNewVmImages(const QStringList& newImageInfo){
    qDebug() << "[II] [SnapTreeModel]" << "addNewVmImages";
    static const QRegularExpression re(R"(-id-(\d{10}))");

    for (int n = 0; n < m_nodes.size(); ++n){
        qDebug() << "[" << n << "]";
        qDebug() << "[id]" <<       m_nodes[n].id;
        qDebug() << "[parentId]" << m_nodes[n].parentId;
        qDebug() << "[name]" <<     m_nodes[n].name;
        qDebug() << "[uuid]" <<     m_nodes[n].uuid;

        if ((m_nodes[n].id == 1) && (m_nodes[n].parentId == -1 )){
            m_nodes[n].imagesFullNames.push_back(newImageInfo.first());
            m_nodes[n].backFullNames.push_back("None");
        }
        else {
            QString localIFullName = m_nodes[n].imagesFullNames.first();
            QString localBFullName = m_nodes[n].backFullNames.first();

            QRegularExpressionMatch imgMatch = re.match(localIFullName);
            QRegularExpressionMatch backingMatch = re.match(localBFullName);

            QString imgUUID =imgMatch.captured(1);
            QString backingUUID = backingMatch.captured(1);

            QString iFullName = getSnapName(newImageInfo.first(), imgUUID);
            QString bFullName = getSnapName(newImageInfo.first(), backingUUID);

            m_nodes[n].imagesFullNames.push_back(iFullName);
            m_nodes[n].backFullNames.push_back(bFullName);
        }
        qDebug() << "- - - - - - - - - - - - -";
    }
}

// *** Формирование имени файла в узлах, аналог SnapManager::getSnapName *** //
QString SnapTreeModel::getSnapName(const QString& imgName, const QString& id){
    QString snapName = imgName;

    if (snapName.endsWith(".qcow2", Qt::CaseInsensitive)) {
        snapName.chop(6);
    }

    snapName += "-id-" + id + ".qcow2";
    return snapName;
}


void SnapTreeModel::delVmImages(const QString& imageFullName){
    qDebug() << "[II] [SnapTreeModel]" << "delVmImages";
}

QModelIndex SnapTreeModel::getActiveStateIndex(){
    QModelIndex res;

    // *** Поиск id узла с imagesType == active *** //
    int activeNodeId = -1;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].imagesType == "active") {
            activeNodeId = m_nodes[i].id;
        }
    }

    if (activeNodeId == -1){
        return res;
    }

    // *** id активного узла получааем ссылку на весь узел *** //
    ChainNode activeNode = findNodeById(activeNodeId);

    // *** Определяем row активного узла среди всех потомков его родителя *** //
    int row = rowOfChild(activeNode.id, activeNode.parentId);

    // *** Создание индекса данного узла *** //
    res  = createIndex(row, 0, activeNode.id);
    return res;
}

void SnapTreeModel::setActive(const QModelIndex& newActiveIndex){

    // *** Поиск id узла с imagesType == active и его сброс до "work" *** //
    QModelIndex prevActiveIndex = getActiveStateIndex();

    int activeNodeId = -1;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].imagesType == "active") {
            m_nodes[i].imagesType = "work";
            break;
        }
    }
    emit dataChanged(prevActiveIndex, prevActiveIndex, {Qt::DisplayRole});

    ChainNode node = getChainNodeByIndex(newActiveIndex);
    // *** Поиск узла и установка флага "active" *** //
    for (int i = 0; i < m_nodes.size(); ++i){
        if (m_nodes[i].id == node.id){
            m_nodes[i].imagesType = "active";
            break;
        }
    }

    emit dataChanged(newActiveIndex, newActiveIndex, {Qt::DisplayRole});
}

// End snapTreeModel.cpp
