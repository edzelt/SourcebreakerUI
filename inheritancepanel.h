#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTreeView;
class QLineEdit;
QT_END_NAMESPACE

class EntityTreeModel;

// ---------------------------------------------------------------------------
// InheritancePanel — левая верхняя панель: дерево наследования классов
//
// Показывает лес классов проекта по иерархии наследования.
// Корни — классы без родителя. Узлы разворачиваются треугольничком.
// Непросмотренные (status=0) — жирным, просмотренные — серым.
// ---------------------------------------------------------------------------
class InheritancePanel : public QWidget
{
    Q_OBJECT

public:
    explicit InheritancePanel(QWidget* parent = nullptr);

    // Перезагрузить дерево из текущей открытой базы
    void reload();

    // Очистить (база закрыта)
    void clear();

signals:
    // Пользователь выбрал класс — передаём его id
    void classSelected(int entityId);

private slots:
    void onSelectionChanged();
    void onFilterChanged(const QString& text);

private:
    void setupUi();

    QLineEdit*       m_filter = nullptr;
    QTreeView*       m_tree   = nullptr;
    EntityTreeModel* m_model  = nullptr;
};