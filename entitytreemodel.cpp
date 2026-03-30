#include "EntityTreeModel.h"
#include "Storage.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFont>
#include <QColor>
#include <QDebug>

static constexpr int kRelInherits  = 2;   // rel_type = Inherits
static constexpr int kKindStruct   = 14;  // CXIdxEntity_Struct
static constexpr int kKindCXXClass = 16;  // CXIdxEntity_CXXClass

// ---------------------------------------------------------------------------
EntityTreeModel::EntityTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_root(new EntityTreeItem(nullptr))
{
    m_root->childrenLoaded = true;
    m_root->hasChildren    = false;
}

EntityTreeModel::~EntityTreeModel() { delete m_root; }

// ---------------------------------------------------------------------------
void EntityTreeModel::clear()
{
    beginResetModel();
    qDeleteAll(m_root->children);
    m_root->children.clear();
    endResetModel();
}

// ---------------------------------------------------------------------------
void EntityTreeModel::reload()
{
    beginResetModel();
    qDeleteAll(m_root->children);
    m_root->children.clear();

    if (Storage::instance().isOpen())
        loadRoots();

    endResetModel();
}

// ---------------------------------------------------------------------------
// Один запрос: корневые классы + флаг наличия потомков
// ---------------------------------------------------------------------------
void EntityTreeModel::loadRoots()
{
    QSqlDatabase db = Storage::instance().database();
    QSqlQuery q(db);

    q.prepare(QStringLiteral(R"(
        SELECT e.id, e.name, e.alias, e.status,
               CASE WHEN ch.to_id IS NOT NULL THEN 1 ELSE 0 END AS has_children
        FROM entities e
        LEFT JOIN relations r
               ON r.from_id = e.id AND r.rel_type = :rel1
        LEFT JOIN (
               SELECT DISTINCT r2.to_id
               FROM relations r2
               JOIN entities ce ON ce.id = r2.from_id AND ce.is_external = 0
               WHERE r2.rel_type = :rel2
        ) ch ON ch.to_id = e.id
        WHERE e.is_external = 0
          AND e.kind IN (:kClass, :kStruct)
          AND r.from_id IS NULL
        ORDER BY e.name
    )"));
    q.bindValue(QStringLiteral(":kClass"),  kKindCXXClass);
    q.bindValue(QStringLiteral(":kStruct"), kKindStruct);
    q.bindValue(QStringLiteral(":rel1"),    kRelInherits);
    q.bindValue(QStringLiteral(":rel2"),    kRelInherits);

    if (!q.exec()) {
        qWarning() << "loadRoots:" << q.lastError().text();
        return;
    }

    while (q.next()) {
        auto* item           = new EntityTreeItem(m_root);
        item->id             = q.value(0).toInt();
        item->name           = q.value(1).toString();
        item->alias          = q.value(2).toString();
        item->status         = q.value(3).toInt();
        item->hasChildren    = q.value(4).toInt() > 0;
        item->childrenLoaded = false;
        m_root->children.append(item);
    }
}

// ---------------------------------------------------------------------------
// Загрузить потомков при разворачивании — один запрос, has_children в нём же
// ---------------------------------------------------------------------------
void EntityTreeModel::loadChildren(EntityTreeItem* item)
{
    if (item->childrenLoaded)
        return;

    QSqlDatabase db = Storage::instance().database();
    QSqlQuery q(db);

    // Один запрос: потомки + флаг наличия их потомков
    q.prepare(QStringLiteral(R"(
        SELECT e.id, e.name, e.alias, e.status,
               CASE WHEN ch.to_id IS NOT NULL THEN 1 ELSE 0 END AS has_children
        FROM entities e
        JOIN relations r ON r.from_id = e.id
        LEFT JOIN (
               SELECT DISTINCT r2.to_id
               FROM relations r2
               JOIN entities ce ON ce.id = r2.from_id AND ce.is_external = 0
               WHERE r2.rel_type = :rel2
        ) ch ON ch.to_id = e.id
        WHERE r.to_id      = :parentId
          AND r.rel_type   = :rel1
          AND e.is_external = 0
        ORDER BY e.name
    )"));
    q.bindValue(QStringLiteral(":parentId"), item->id);
    q.bindValue(QStringLiteral(":rel1"),     kRelInherits);
    q.bindValue(QStringLiteral(":rel2"),     kRelInherits);

    if (!q.exec()) {
        qWarning() << "loadChildren:" << q.lastError().text();
        item->childrenLoaded = true;
        return;
    }

    while (q.next()) {
        auto* child           = new EntityTreeItem(item);
        child->id             = q.value(0).toInt();
        child->name           = q.value(1).toString();
        child->alias          = q.value(2).toString();
        child->status         = q.value(3).toInt();
        child->hasChildren    = q.value(4).toInt() > 0;
        child->childrenLoaded = false;
        item->children.append(child);
    }

    item->childrenLoaded = true;
}

// ---------------------------------------------------------------------------
EntityTreeItem* EntityTreeModel::itemFromIndex(const QModelIndex& index) const
{
    if (!index.isValid())
        return m_root;
    return static_cast<EntityTreeItem*>(index.internalPointer());
}

// ---------------------------------------------------------------------------
// hasChildren — Qt спрашивает показывать ли треугольничек
// ---------------------------------------------------------------------------
bool EntityTreeModel::hasChildren(const QModelIndex& parent) const
{
    auto* item = itemFromIndex(parent);
    if (item == m_root)
        return !m_root->children.isEmpty();
    // Если дети загружены — смотрим реальный список
    if (item->childrenLoaded)
        return !item->children.isEmpty();
    // Иначе — флаг из запроса
    return item->hasChildren;
}

// ---------------------------------------------------------------------------
// canFetchMore / fetchMore — ленивая загрузка при разворачивании
// ---------------------------------------------------------------------------
bool EntityTreeModel::canFetchMore(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return false;
    auto* item = itemFromIndex(parent);
    return item->hasChildren && !item->childrenLoaded;
}

void EntityTreeModel::fetchMore(const QModelIndex& parent)
{
    if (!parent.isValid())
        return;

    auto* item = itemFromIndex(parent);
    if (item->childrenLoaded)
        return;

    loadChildren(item);

    if (!item->children.isEmpty()) {
        beginInsertRows(parent, 0, item->children.size() - 1);
        endInsertRows();
    }
}

// ---------------------------------------------------------------------------
// rowCount — только загруженные дети, пустых строк нет
// ---------------------------------------------------------------------------
int EntityTreeModel::rowCount(const QModelIndex& parent) const
{
    auto* item = itemFromIndex(parent);
    // Незагруженные дети — возвращаем 0, Qt покажет треугольничек
    // через hasChildren и загрузит через fetchMore
    if (!item->childrenLoaded)
        return 0;
    return item->children.size();
}

int EntityTreeModel::columnCount(const QModelIndex&) const { return 1; }

// ---------------------------------------------------------------------------
QModelIndex EntityTreeModel::index(int row, int column,
                                   const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return {};
    auto* parentItem = itemFromIndex(parent);
    if (row < 0 || row >= parentItem->children.size())
        return {};
    return createIndex(row, column, parentItem->children[row]);
}

QModelIndex EntityTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return {};
    auto* item       = itemFromIndex(index);
    auto* parentItem = item->parent;
    if (!parentItem || parentItem == m_root)
        return {};
    auto* grandParent = parentItem->parent;
    if (!grandParent)
        return {};
    int row = grandParent->children.indexOf(parentItem);
    if (row < 0)
        return {};
    return createIndex(row, 0, parentItem);
}

// ---------------------------------------------------------------------------
QVariant EntityTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};
    auto* item = itemFromIndex(index);

    switch (role) {
    case Qt::DisplayRole:
        return item->displayName();
    case Qt::ToolTipRole:
        if (!item->alias.isEmpty())
            return QStringLiteral("%1\n(оригинал: %2)")
                .arg(item->alias, item->name);
        return item->name;
    case Qt::ForegroundRole:
        if (item->status > 0)
            return QColor(Qt::gray);
        return {};
    case Qt::FontRole:
        if (item->status == 0) {
            QFont f;
            f.setBold(true);
            return f;
        }
        return {};
    case Qt::UserRole:
        return item->id;
    default:
        return {};
    }
}

QVariant EntityTreeModel::headerData(int section,
                                     Qt::Orientation orientation,
                                     int role) const
{
    if (orientation == Qt::Horizontal
        && role == Qt::DisplayRole
        && section == 0)
        return tr("Иерархия классов");
    return {};
}