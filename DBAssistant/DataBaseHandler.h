#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H
#include "DBADefine.h"
#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

// 数据库控制基类
class DataBaseHandler : public QObject
{
public:
    DataBaseHandler() = default;
    ~DataBaseHandler() = default;
    virtual bool Open(const QString& driver, const QString& dbName, const QString& host, int port, const QString& user, const QString& password);
    virtual bool Open() = 0;
    virtual bool Close() = 0;
    virtual bool IsEnable() = 0;
    virtual bool ExeSql(const QString& sql, QString& error) = 0;
    virtual bool QuerySql(const QString& sql, QSqlQuery& query, QString& error) = 0;
    QSqlDatabase& GetSqlDatabase();
    void SetSqlDatabase(const QSqlDatabase& db);

private:
    QSqlDatabase m_db;
};

#endif // DATABASEHANDLER_H
