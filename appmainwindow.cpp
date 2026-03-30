#include "AppMainWindow.h"
#include "Storage.h"
#include "InheritancePanel.h"

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileInfo>
#include <QFrame>
#include <QStyle>

// ---------------------------------------------------------------------------
AppMainWindow::AppMainWindow(QWidget* parent)
    : KDDockWidgets::QtWidgets::MainWindow(
          QStringLiteral("SourcebreakerMainWindow"), {}, parent)
{
    QWidget::setMinimumSize(1100, 700);

    setupMenu();
    setupToolBar();
    setupStatusBar();
    setupDocks();

    updateWindowState();
}

AppMainWindow::~AppMainWindow() = default;

// ---------------------------------------------------------------------------
void AppMainWindow::setupDocks()
{
    m_inheritancePanel = new InheritancePanel(this);

    auto* dock = new KDDockWidgets::QtWidgets::DockWidget(
        QStringLiteral("Иерархия классов"));
    dock->setWidget(m_inheritancePanel);
    dock->setTitle(tr("Иерархия классов"));

    addDockWidget(dock, KDDockWidgets::Location_OnLeft);

    connect(m_inheritancePanel, &InheritancePanel::classSelected,
            this, &AppMainWindow::onClassSelected);
}

// ---------------------------------------------------------------------------
void AppMainWindow::setupMenu()
{
    QMenu* menuFile = menuBar()->addMenu(tr("&Файл"));

    m_actOpen = menuFile->addAction(
        style()->standardIcon(QStyle::SP_DialogOpenButton),
        tr("&Открыть базу..."),
        QKeySequence::Open,
        this, &AppMainWindow::onOpenDatabase);

    m_actClose = menuFile->addAction(
        style()->standardIcon(QStyle::SP_DialogCloseButton),
        tr("&Закрыть базу"),
        QKeySequence(Qt::CTRL | Qt::Key_W),
        this, &AppMainWindow::onCloseDatabase);

    menuFile->addSeparator();

    m_actExit = menuFile->addAction(
        style()->standardIcon(QStyle::SP_DialogCloseButton),
        tr("В&ыход"),
        QKeySequence::Quit,
        this, &AppMainWindow::onExit);

    QMenu* menuHelp = menuBar()->addMenu(tr("&Справка"));
    m_actAbout = menuHelp->addAction(
        style()->standardIcon(QStyle::SP_MessageBoxInformation),
        tr("&О программе"),
        this, &AppMainWindow::onAbout);
}

// ---------------------------------------------------------------------------
void AppMainWindow::setupToolBar()
{
    QToolBar* tb = addToolBar(tr("Основная панель"));
    tb->setObjectName(QStringLiteral("MainToolBar"));
    tb->setMovable(false);
    tb->setIconSize(QSize(22, 22));
    tb->addAction(m_actOpen);
    tb->addAction(m_actClose);
    tb->addSeparator();
}

// ---------------------------------------------------------------------------
void AppMainWindow::setupStatusBar()
{
    QStatusBar* sb = statusBar();

    m_statusDbPath = new QLabel(tr("База не открыта"), this);
    m_statusDbPath->setMinimumWidth(200);
    sb->addWidget(m_statusDbPath, 1);

    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::VLine);
    sep1->setFrameShadow(QFrame::Sunken);
    sb->addPermanentWidget(sep1);

    m_statusIndexState = new QLabel(this);
    sb->addPermanentWidget(m_statusIndexState);

    auto* sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::VLine);
    sep2->setFrameShadow(QFrame::Sunken);
    sb->addPermanentWidget(sep2);

    m_statusEntityCount = new QLabel(this);
    m_statusEntityCount->setMinimumWidth(140);
    m_statusEntityCount->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    sb->addPermanentWidget(m_statusEntityCount);
}

// ---------------------------------------------------------------------------
void AppMainWindow::updateWindowState()
{
    Storage& db = Storage::instance();

    if (!db.isOpen()) {
        setWindowTitle(tr("SourcebreakerUI"));
        m_actClose->setEnabled(false);
        m_statusDbPath->setText(tr("База не открыта"));
        m_statusIndexState->setText({});
        m_statusEntityCount->setText({});
        m_inheritancePanel->clear();
        return;
    }

    setWindowTitle(
        QStringLiteral("%1 — SourcebreakerUI").arg(db.projectName()));
    m_actClose->setEnabled(true);
    m_statusDbPath->setText(db.currentPath());

    if (db.isIndexingComplete()) {
        m_statusIndexState->setText(tr("✔ Индексация завершена"));
        m_statusIndexState->setStyleSheet(
            QStringLiteral("color: green; font-weight: bold;"));
    } else {
        m_statusIndexState->setText(tr("⚠ Индексация не завершена"));
        m_statusIndexState->setStyleSheet(
            QStringLiteral("color: orange; font-weight: bold;"));
    }

    m_statusEntityCount->setText(
        tr("Сущностей: %1").arg(QLocale().toString(db.entityCount())));

    m_inheritancePanel->reload();
}

// ---------------------------------------------------------------------------
void AppMainWindow::openDatabaseAt(const QString& path)
{
    if (Storage::instance().isOpen())
        Storage::instance().close();

    QString error = Storage::instance().open(path);
    if (!error.isEmpty()) {
        QMessageBox::critical(this, tr("Ошибка открытия базы"), error);
        updateWindowState();
        return;
    }

    updateWindowState();
    statusBar()->showMessage(tr("База открыта: %1").arg(path), 3000);
}

// ---------------------------------------------------------------------------
void AppMainWindow::onClassSelected(int entityId)
{
    statusBar()->showMessage(tr("Выбран класс id=%1").arg(entityId), 2000);
}

void AppMainWindow::onOpenDatabase()
{
    QFileDialog dlg(this, tr("Открыть базу данных"));
    dlg.setNameFilter(tr("Базы Sourcebreaker (*.db);;Все файлы (*)"));
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);

    if (Storage::instance().isOpen()) {
        QFileInfo fi(Storage::instance().currentPath());
        dlg.setDirectory(fi.absolutePath());
    }

    if (dlg.exec() != QDialog::Accepted)
        return;

    const QStringList files = dlg.selectedFiles();
    if (!files.isEmpty())
        openDatabaseAt(files.first());
}

void AppMainWindow::onCloseDatabase()
{
    if (!Storage::instance().isOpen()) return;
    Storage::instance().close();
    updateWindowState();
    statusBar()->showMessage(tr("База закрыта"), 2000);
}

void AppMainWindow::onExit() { close(); }

void AppMainWindow::onAbout()
{
    QMessageBox::about(this,
                       tr("О программе SourcebreakerUI"),
                       tr("<b>SourcebreakerUI</b><br/>"
                          "Редактор баз данных анализа C++ проектов.<br/><br/>"
                          "Стек: Qt 6 · KDDockWidgets · SQLite · libclang<br/>"
                          "MSVC 2022 · Windows"));
}

void AppMainWindow::closeEvent(QCloseEvent* event)
{
    if (Storage::instance().isOpen())
        Storage::instance().close();
    event->accept();
}