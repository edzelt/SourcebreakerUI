#pragma once

#include <kddockwidgets/qtwidgets/views/MainWindow.h>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
QT_END_NAMESPACE

class InheritancePanel;

// ---------------------------------------------------------------------------
// AppMainWindow — главное окно SourcebreakerUI
//
// Переименовано из MainWindow чтобы избежать конфликта имён с
// KDDockWidgets::QtWidgets::MainWindow из которого наследуемся.
// ---------------------------------------------------------------------------
class AppMainWindow : public KDDockWidgets::QtWidgets::MainWindow
{
    Q_OBJECT

public:
    explicit AppMainWindow(QWidget* parent = nullptr);
    ~AppMainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onOpenDatabase();
    void onCloseDatabase();
    void onExit();
    void onAbout();
    void onClassSelected(int entityId);

private:
    void setupMenu();
    void setupToolBar();
    void setupStatusBar();
    void setupDocks();

    void updateWindowState();
    void openDatabaseAt(const QString& path);

    QAction* m_actOpen  = nullptr;
    QAction* m_actClose = nullptr;
    QAction* m_actExit  = nullptr;
    QAction* m_actAbout = nullptr;

    QLabel* m_statusDbPath      = nullptr;
    QLabel* m_statusEntityCount = nullptr;
    QLabel* m_statusIndexState  = nullptr;

    InheritancePanel* m_inheritancePanel = nullptr;
};