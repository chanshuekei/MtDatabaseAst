#ifndef MYSQLHANDLER_H
#define MYSQLHANDLER_H
#include "DataBaseHandler.h"
#include <QSqlDatabase>

class MysqlHandler : public DataBaseHandler
{
public:
    MysqlHandler();
    ~MysqlHandler();
public:
    virtual bool Open() override;
    virtual bool Close() override;
    virtual bool IsEnable() override;
    virtual bool ExeSql(const QString& sql, QString& error) override;
    virtual bool QuerySql(const QString& sql, QSqlQuery& query, QString& error) override;
};

#endif // MYSQLHANDLER_H
