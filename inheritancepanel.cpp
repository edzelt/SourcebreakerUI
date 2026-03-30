#include "InheritancePanel.h"
#include "EntityTreeModel.h"

#include <QTreeView>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <QHeaderView>

// ---------------------------------------------------------------------------
InheritancePanel::InheritancePanel(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

// ---------------------------------------------------------------------------
void InheritancePanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    // Строка фильтрации
    m_filter = new QLineEdit(this);
    m_filter->setPlaceholderText(tr("Фильтр классов..."));
    m_filter->setClearButtonEnabled(true);
    layout->addWidget(m_filter);

    // Дерево
    m_tree  = new QTreeView(this);
    m_model = new EntityTreeModel(this);

    // Прокси-модель для фильтрации по имени
    auto* proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(m_model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterKeyColumn(0);
    proxy->setRecursiveFilteringEnabled(true); // Qt 5.10+

    m_tree->setModel(proxy);
    m_tree->setHeaderHidden(false);
    m_tree->header()->setStretchLastSection(true);
    m_tree->setUniformRowHeights(true);    // быстрее для больших деревьев
    m_tree->setAnimated(true);             // плавное раскрытие узлов
    m_tree->setAlternatingRowColors(true);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(m_tree);

    // Сигналы
    connect(m_filter, &QLineEdit::textChanged,
            this, &InheritancePanel::onFilterChanged);

    connect(m_tree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &InheritancePanel::onSelectionChanged);
}

// ---------------------------------------------------------------------------
void InheritancePanel::reload()
{
    m_filter->clear();
    m_model->reload();
    m_tree->expandToDepth(0); // раскрыть первый уровень сразу
}

// ---------------------------------------------------------------------------
void InheritancePanel::clear()
{
    m_filter->clear();
    m_model->clear();
}

// ---------------------------------------------------------------------------
void InheritancePanel::onSelectionChanged()
{
    const QModelIndexList selected =
        m_tree->selectionModel()->selectedIndexes();

    if (selected.isEmpty())
        return;

    // Прокси-индекс → индекс исходной модели → id сущности
    auto* proxy = static_cast<QSortFilterProxyModel*>(m_tree->model());
    QModelIndex sourceIndex = proxy->mapToSource(selected.first());

    int entityId = m_model->data(sourceIndex, Qt::UserRole).toInt();
    if (entityId > 0)
        emit classSelected(entityId);
}

// ---------------------------------------------------------------------------
void InheritancePanel::onFilterChanged(const QString& text)
{
    auto* proxy = static_cast<QSortFilterProxyModel*>(m_tree->model());
    proxy->setFilterFixedString(text);

    // При фильтрации раскрываем всё чтобы были видны совпадения
    if (!text.isEmpty())
        m_tree->expandAll();
    else
        m_tree->expandToDepth(0);
}