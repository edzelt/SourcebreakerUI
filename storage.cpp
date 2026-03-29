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

// ---------------------------------------------------------------------------
Storage::~Storage()
{
    close();
}

// ---------------------------------------------------------------------------
QString Storage::open(const QString& path)
{
    // Закрыть предыдущую базу, если была открыта
    close();

    // Проверить существование файла
    if (!QFileInfo::exists(path))
        return QStringLiteral("Файл не найден: %1").arg(path);

    // Создать соединение
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), kConnectionName);
    m_db.setDatabaseName(path);
    // Открываем только для чтения и редактирования пользовательских данных;
    // индексатор — отдельная утилита
    m_db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY=0"));

    if (!m_db.open()) {
        QString err = m_db.lastError().text();
        QSqlDatabase::removeDatabase(kConnectionName);
        m_db = QSqlDatabase();
        return QStringLiteral("Не удалось открыть базу данных: %1").arg(err);
    }

    // Проверить структуру базы
    QString schemaError;
    if (!validateSchema(schemaError)) {
        close();
        return QStringLiteral("Файл не является базой Sourcebreaker: %1").arg(schemaError);
    }

    m_path = path;
    return {}; // Успех
}

// ---------------------------------------------------------------------------
void Storage::close()
{
    if (m_db.isOpen())
        m_db.close();

    // Удалить именованное соединение из пула Qt
    if (QSqlDatabase::contains(kConnectionName))
        QSqlDatabase::removeDatabase(kConnectionName);

    m_db   = QSqlDatabase();
    m_path.clear();
}

// ---------------------------------------------------------------------------
bool Storage::isOpen() const
{
    return m_db.isValid() && m_db.isOpen();
}

// ---------------------------------------------------------------------------
QString Storage::currentPath() const
{
    return m_path;
}

// ---------------------------------------------------------------------------
QString Storage::projectName() const
{
    if (!isOpen())
        return {};

    // Предпочитаем project_root из метаданных
    if (auto root = dbInfo(QStringLiteral("project_root"))) {
        // Берём последнюю компоненту пути как имя проекта
        QFileInfo fi(*root);
        QString name = fi.fileName();
        if (!name.isEmpty())
            return name;
    }

    // Запасной вариант — имя файла базы без расширения
    return QFileInfo(m_path).baseName();
}

// ---------------------------------------------------------------------------
bool Storage::isIndexingComplete() const
{
    auto val = dbInfo(QStringLiteral("indexing_complete"));
    return val.has_value() && *val == QStringLiteral("1");
}

// ---------------------------------------------------------------------------
std::optional<QString> Storage::dbInfo(const QString& key) const
{
    if (!isOpen())
        return std::nullopt;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT value FROM db_info WHERE key = :key"));
    q.bindValue(QStringLiteral(":key"), key);

    if (!q.exec() || !q.next())
        return std::nullopt;

    return q.value(0).toString();
}

// ---------------------------------------------------------------------------
int Storage::entityCount() const
{
    if (!isOpen())
        return 0;

    QSqlQuery q(m_db);
    if (!q.exec(QStringLiteral("SELECT COUNT(*) FROM entities")))
        return 0;

    return q.next() ? q.value(0).toInt() : 0;
}

// ---------------------------------------------------------------------------
bool Storage::validateSchema(QString& outError) const
{
    // Список обязательных таблиц схемы Sourcebreaker
    const QStringList required = {
        QStringLiteral("db_info"),
        QStringLiteral("entities"),
        QStringLiteral("relations"),
        QStringLiteral("entity_bodies"),
        QStringLiteral("local_entities"),
        QStringLiteral("groups"),
        QStringLiteral("group_members"),
    };

    QStringList existing = m_db.tables();

    for (const QString& table : required) {
        if (!existing.contains(table)) {
            outError = QStringLiteral("отсутствует таблица \"%1\"").arg(table);
            return false;
        }
    }

    return true;
}