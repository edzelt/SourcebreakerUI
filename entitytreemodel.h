#pragma once

#include <QAbstractItemModel>
#include <QVector>
#include <QString>

// ---------------------------------------------------------------------------
// EntityTreeItem — узел дерева наследования
// ---------------------------------------------------------------------------
struct EntityTreeItem
{
    int     id         = 0;
    QString name;
    QString alias;
    int     status     = 0;

    EntityTreeItem*          parent   = nullptr;
    QVector<EntityTreeItem*> children;

    // Флаг ленивой загрузки: дочерние ещё не загружены
    bool childrenLoaded = false;

    // Есть ли вообще потомки (определяется одним быстрым COUNT запросом)
    bool hasChildren    = false;

    explicit EntityTreeItem(EntityTreeItem* parent = nullptr)
        : parent(parent) {}

    ~EntityTreeItem() { qDeleteAll(children); }

    QString displayName() const
    {
        return alias.isEmpty() ? name : alias;
    }
};

// ---------------------------------------------------------------------------
// EntityTreeModel — дерево классов по иерархии наследования
//
// Ленивая загрузка: потомки загружаются только при разворачивании узла.
// Корни загружаются одним запросом при reload().
// ---------------------------------------------------------------------------
class EntityTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit EntityTreeModel(QObject* parent = nullptr);
    ~EntityTreeModel() override;

    void reload();
    void clear();

    EntityTreeItem* itemFromIndex(const QModelIndex& index) const;

    QModelIndex   index(int row, int column,
                      const QModelIndex& parent = {}) const override;
    QModelIndex   parent(const QModelIndex& index) const override;
    int           rowCount(const QModelIndex& parent = {}) const override;
    int           columnCount(const QModelIndex& parent = {}) const override;
    QVariant      data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;
    QVariant      headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    bool          hasChildren(const QModelIndex& parent = {}) const override;

    // Вызывается QTreeView при разворачивании узла
    void          fetchMore(const QModelIndex& parent) override;
    bool          canFetchMore(const QModelIndex& parent) const override;

private:
    void loadRoots();
    void loadChildren(EntityTreeItem* item);
    bool queryHasChildren(int entityId) const;

    EntityTreeItem* m_root = nullptr;
};