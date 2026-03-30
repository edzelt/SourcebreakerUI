#include "Storage.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>

// ---------------------------------------------------------------------------
Storage& Storage::instance()
{
    static Storage s;
    return s;
}

Storage::~Storage() { close(); }

// ---------------------------------------------------------------------------
QString Storage::open(const QString& path)
{
    close();

    if (!QFileInfo::exists(path))
        return QStringLiteral("Файл не найден: %1").arg(path);

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), kConnectionName);
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        QString err = m_db.lastError().text();
        QSqlDatabase::removeDatabase(kConnectionName);
        m_db = QSqlDatabase();
        return QStringLiteral("Не удалось открыть базу: %1").arg(err);
    }

    QString schemaError;
    if (!validateSchema(schemaError)) {
        close();
        return QStringLiteral("Файл не является базой Sourcebreaker: %1").arg(schemaError);
    }

    m_path = path;
    return {};
}

// ---------------------------------------------------------------------------
void Storage::close()
{
    if (m_db.isOpen())
        m_db.close();
    if (QSqlDatabase::contains(kConnectionName))
        QSqlDatabase::removeDatabase(kConnectionName);
    m_db = QSqlDatabase();
    m_path.clear();
}

// ---------------------------------------------------------------------------
bool    Storage::isOpen()      const { return m_db.isValid() && m_db.isOpen(); }
QString Storage::currentPath() const { return m_path; }

// ---------------------------------------------------------------------------
QString Storage::projectName() const
{
    if (!isOpen()) return {};

    // Пробуем взять из db_info — таблица может отсутствовать в старых базах
    if (m_db.tables().contains(QStringLiteral("db_info"))) {
        if (auto root = dbInfo(QStringLiteral("project_root"))) {
            QString name = QFileInfo(*root).fileName();
            if (!name.isEmpty()) return name;
        }
    }
    return QFileInfo(m_path).baseName();
}

// ---------------------------------------------------------------------------
bool Storage::isIndexingComplete() const
{
    if (!m_db.tables().contains(QStringLiteral("db_info")))
        return true; // старая база без db_info — считаем завершённой
    auto val = dbInfo(QStringLiteral("indexing_complete"));
    return val.has_value() && *val == QStringLiteral("1");
}

// ---------------------------------------------------------------------------
std::optional<QString> Storage::dbInfo(const QString& key) const
{
    if (!isOpen()) return std::nullopt;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT value FROM db_info WHERE key = :k"));
    q.bindValue(QStringLiteral(":k"), key);
    if (!q.exec() || !q.next()) return std::nullopt;
    return q.value(0).toString();
}

// ---------------------------------------------------------------------------
int Storage::entityCount() const
{
    if (!isOpen()) return 0;
    QSqlQuery q(m_db);
    // Считаем только внутренние сущности проекта
    if (!q.exec(QStringLiteral("SELECT COUNT(*) FROM entities WHERE is_external=0")))
        return 0;
    return q.next() ? q.value(0).toInt() : 0;
}

// ---------------------------------------------------------------------------
bool Storage::validateSchema(QString& outError) const
{
    // Обязательные таблицы — без db_info, она появилась в новых версиях
    const QStringList required = {
        QStringLiteral("entities"),
        QStringLiteral("relations"),
        QStringLiteral("entity_bodies"),
        QStringLiteral("local_entities"),
        QStringLiteral("groups"),
        QStringLiteral("group_members"),
    };

    const QStringList existing = m_db.tables();
    for (const QString& table : required) {
        if (!existing.contains(table)) {
            outError = QStringLiteral("отсутствует таблица \"%1\"").arg(table);
            return false;
        }
    }
    return true;
}