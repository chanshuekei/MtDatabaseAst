#ifndef DBADEFINE_H
#define DBADEFINE_H
#include <QString>

#pragma execution_character_set("utf-8")

namespace TKDBA_SPACE
{
    // 数据库类型
    enum class DATABASE_TYPE { SQLITE_TYPE, ORACLE_TYPE, MYSQL_TYPE, };
    // 参数
    struct stDatabaseParamter
    {
        stDatabaseParamter()
        {

        };

        QString Host;
        int Port { 0 };
        QString User;
        QString Password;
        QString Database;
    };
    using DatabaseParams = stDatabaseParamter;
}


#endif // DBADEFINE_H
