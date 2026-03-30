#pragma once

#include <QString>
#include <QSqlDatabase>
#include <optional>

// ---------------------------------------------------------------------------
// Storage — подключение к базе данных Sourcebreaker (.db)
//
// Синглтон. UI работает только в режиме чтения и редактирования
// пользовательских полей (alias, short_note, description, status).
// Индексатор — отдельная утилита, создающая базу.
// ---------------------------------------------------------------------------
class Storage
{
public:
    static Storage& instance();

    // Открыть базу. Возвращает пустую строку при успехе, иначе ошибку.
    QString open(const QString& path);

    void    close();
    bool    isOpen()      const;
    QString currentPath() const;

    // Имя проекта из db_info(project_root) или имя файла базы
    QString projectName() const;

    // Индексация завершена (indexing_complete = "1")
    bool isIndexingComplete() const;

    // Значение из таблицы db_info
    std::optional<QString> dbInfo(const QString& key) const;

    // Количество внутренних сущностей (is_external=0) — для StatusBar
    int entityCount() const;

    // Прямой доступ к соединению для выполнения запросов из панелей UI
    QSqlDatabase database() const { return m_db; }

private:
    Storage()  = default;
    ~Storage();
    Storage(const Storage&)            = delete;
    Storage& operator=(const Storage&) = delete;

    bool validateSchema(QString& outError) const;

    QString      m_path;
    QSqlDatabase m_db;

    static constexpr const char* kConnectionName = "sourcebreaker_ui_main";
};