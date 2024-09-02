#include "DBHandlerFactory.h"
#include "SqliteHandler.h"
#include "OracleHandler.h"
#include "MysqlHandler.h"
#include "easylogging++.h"
#include <QString>
#pragma execution_character_set("utf-8")
using namespace TKDBA_SPACE;

#ifndef X
#define X(log) QString::fromLocal8Bit(log)
#endif

DataBaseHandler* DBHandlerFactory::CreateHandler(const TKDBA_SPACE::DATABASE_TYPE type)
{
    DataBaseHandler* handler = nullptr;
    switch(type)
    {
        case DATABASE_TYPE::SQLITE_TYPE:
            LOG(INFO) << X("����SQLITE�����ģ��!");
            handler = new SqliteHandler();
            break;
        case DATABASE_TYPE::ORACLE_TYPE:
            LOG(INFO) << X("����ORACLE�����ģ��!");
            handler = new OracleHandler();
            break;
        case DATABASE_TYPE::MYSQL_TYPE:
            LOG(INFO) << X("����MYSQL�����ģ��!");
            handler = new MysqlHandler();
            break;
        default:
            LOG(INFO) << X("�������ݿ����ģ��ʧ��!");
            break;
    }

    return handler;
}

TKDBA_SPACE::DATABASE_TYPE DBHandlerFactory::ConvertInt2DbType(const int& ini)
{
    switch(ini)
    {
        case 0:
            return TKDBA_SPACE::DATABASE_TYPE::SQLITE_TYPE;
        case 1:
            return TKDBA_SPACE::DATABASE_TYPE::ORACLE_TYPE;
        case 2:
            return TKDBA_SPACE::DATABASE_TYPE::MYSQL_TYPE;
        default:
            break;
    }

    // Ĭ�Ϸ���SQLITE
    return TKDBA_SPACE::DATABASE_TYPE::SQLITE_TYPE;
}

TKDBA_SPACE::DATABASE_TYPE DBHandlerFactory::ConvertStr2DbType(const QString& ini)
{
    static QMap<QString, TKDBA_SPACE::DATABASE_TYPE> TypeMap =
    {
        {"QSQLITE", TKDBA_SPACE::DATABASE_TYPE::SQLITE_TYPE},
        {"QOCI", TKDBA_SPACE::DATABASE_TYPE::ORACLE_TYPE},
        {"QMYSQL", TKDBA_SPACE::DATABASE_TYPE::MYSQL_TYPE},
    };
    auto finder = TypeMap.find(ini.toUpper());
    if(finder != TypeMap.end())
    {
        return finder.value();
    }
    // Ĭ�Ϸ���SQLITE
    return TKDBA_SPACE::DATABASE_TYPE::SQLITE_TYPE;
}
