#pragma once

#include <QString>
#include <QSqlDatabase>
#include <optional>

// ---------------------------------------------------------------------------
// Storage — управляет подключением к базе данных Sourcebreaker (.db)
//
// Принцип: один экземпляр на всё приложение, доступен через Storage::instance().
// Главная программа работает только в режиме чтения и редактирования
// пользовательских данных — индексатор запускается отдельно.
// ---------------------------------------------------------------------------
class Storage
{
public:
    // Получить единственный экземпляр
    static Storage& instance();

    // Открыть базу данных по пути к файлу .db
    // Возвращает пустую строку при успехе, иначе — описание ошибки
    QString open(const QString& path);

    // Закрыть текущую базу данных
    void close();

    // Проверить, открыта ли база
    bool isOpen() const;

    // Путь к текущему файлу базы данных
    QString currentPath() const;

    // Имя проекта (project_root из таблицы db_info) или имя файла
    QString projectName() const;

    // Проверить, завершена ли индексация (indexing_complete = "1")
    bool isIndexingComplete() const;

    // Читать значение из таблицы db_info
    std::optional<QString> dbInfo(const QString& key) const;

    // Количество сущностей в базе (для StatusBar)
    int entityCount() const;

private:
    Storage() = default;
    ~Storage();

    // Некопируемый синглтон
    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;

    // Проверить схему базы: наличие обязательных таблиц
    bool validateSchema(QString& outError) const;

    QString      m_path;
    QSqlDatabase m_db;

    // Имя соединения — уникальный идентификатор для QSqlDatabase
    static constexpr const char* kConnectionName = "sourcebreaker_main";
};