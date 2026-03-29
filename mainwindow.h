#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
QT_END_NAMESPACE

// ---------------------------------------------------------------------------
// MainWindow — главное окно Sourcebreaker
//
// Создаётся без .ui файла — вся компоновка в коде.
// Использует KDDockWidgets для докинга панелей (подключение настраивается
// при добавлении первых панелей; пока — заготовка главного окна).
// ---------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    // Подтверждение закрытия окна при открытой базе
    void closeEvent(QCloseEvent* event) override;

private slots:
    // Действия меню «Файл»
    void onOpenDatabase();
    void onCloseDatabase();
    void onExit();

    // Действие меню «Справка»
    void onAbout();

private:
    // Инициализация частей интерфейса
    void setupMenu();
    void setupToolBar();
    void setupStatusBar();
    void setupCentralWidget();

    // Обновить заголовок окна и статусную строку после смены базы
    void updateWindowState();

    // Открыть базу данных по конкретному пути (вызывается из onOpenDatabase)
    void openDatabaseAt(const QString& path);

    // --- Действия (QAction) ---
    QAction* m_actOpen   = nullptr;
    QAction* m_actClose  = nullptr;
    QAction* m_actExit   = nullptr;
    QAction* m_actAbout  = nullptr;

    // --- Статусная строка ---
    QLabel* m_statusDbPath      = nullptr; // Путь к файлу базы
    QLabel* m_statusEntityCount = nullptr; // Количество сущностей
    QLabel* m_statusIndexState  = nullptr; // Состояние индексации
};