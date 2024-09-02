#include "MysqlHandler.h"
#include "IniHandler.h"
#include "easylogging++.h"
#pragma execution_character_set("utf-8")

#ifndef X
#define X(log) QString::fromLocal8Bit(log)
#endif

MysqlHandler::MysqlHandler()
{}

MysqlHandler::~MysqlHandler()
{}

bool MysqlHandler::Open()
{
    // 读取配置文件信息
    auto host = IniHandler::GetInstance().GetValue("dbassistant", "Database", "Host", "127.0.0.1");
    auto port = IniHandler::GetInstance().GetValue("dbassistant", "Database", "Port", "1521").toInt();
    auto user = IniHandler::GetInstance().GetValue("dbassistant", "Database", "User");
    auto password = IniHandler::GetInstance().GetValue("dbassistant", "Database", "Password");
    auto database = IniHandler::GetInstance().GetValue("dbassistant", "Database", "DbName", "ORCL");
    // 连接数据库
    auto db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(host);
    db.setPort(port);
    db.setUserName(user);
    db.setPassword(password);
    db.setDatabaseName(database);
    LOG(INFO) << X("打开数据库:%1,%2,%3,%4,%5,%6")
        .arg(host)
        .arg(port)
        .arg(user)
        .arg(password)
        .arg(database)
        .arg(db.driverName());
    if(!db.open())
    {
        LOG(ERROR) << X("打开数据失败:") << db.lastError().text();
        return false;
    }
    else
    {
        LOG(INFO) << X("打开数据成功!");
        SetSqlDatabase(db);
        return true;
    }
}

bool MysqlHandler::Close()
{
    auto db = GetSqlDatabase();
    if(db.isOpen())
    {
        LOG(INFO) << X("关闭数据库:") << db.databaseName();
        db.close();
    }
    return true;
}

bool MysqlHandler::IsEnable()
{
    auto db = GetSqlDatabase();
    return db.isOpen();
}

bool MysqlHandler::ExeSql(const QString& sql, QString& error)
{
    auto db = GetSqlDatabase();
    QSqlQuery query(db);
    if(query.exec(sql))
    {
        error = X("NO ERROR");
        return true;
    }
    else
    {
        error = query.lastError().text();
        return false;
    }
}

bool MysqlHandler::QuerySql(const QString& sql, QSqlQuery& query, QString& error)
{
    auto db = GetSqlDatabase();
    query = QSqlQuery(db);
    if(query.exec(sql))
    {
        error = X("NO ERROR");
        return true;
    }
    else
    {
        error = query.lastError().text();
        return false;
    }
}

