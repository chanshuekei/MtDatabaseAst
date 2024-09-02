#ifndef SQLITEHANDLER_H
#define SQLITEHANDLER_H
#include "DataBaseHandler.h"


class SqliteHandler : public DataBaseHandler
{
public:
    SqliteHandler();
    ~SqliteHandler();
public:
    virtual bool Open() override;
    virtual bool Close() override;
    virtual bool IsEnable() override;
    virtual bool ExeSql(const QString& sql, QString& error) override;
    virtual bool QuerySql(const QString& sql, QSqlQuery& query, QString& error) override;
};

#endif // SQLITEHANDLER_H
