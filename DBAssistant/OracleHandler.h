#ifndef ORACLEHANDLER_H
#define ORACLEHANDLER_H
#include "DataBaseHandler.h"

class OracleHandler : public DataBaseHandler
{
public:
    OracleHandler();
    ~OracleHandler();
public:
    virtual bool Open() override;
    virtual bool Close() override;
    virtual bool IsEnable() override;
    virtual bool ExeSql(const QString& sql, QString& error) override;
    virtual bool QuerySql(const QString& sql, QSqlQuery& query, QString& error) override;
};

#endif // ORACLEHANDLER_H
