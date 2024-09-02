#include "OracleHandler.h"
#include "IniHandler.h"
#include "easylogging++.h"
#pragma execution_character_set("utf-8")

#ifndef X
#define X(log) QString::fromLocal8Bit(log)
#endif

OracleHandler::OracleHandler()
{}

OracleHandler::~OracleHandler()
{}

bool OracleHandler::Open()
{
    // ��ȡ�����ļ���Ϣ
    auto host = IniHandler::GetInstance().GetValue("dbassistant", "Database", "Host", "127.0.0.1");
    auto port = IniHandler::GetInstance().GetValue("dbassistant", "Database", "Port", "1521").toInt();
    auto user = IniHandler::GetInstance().GetValue("dbassistant", "Database", "User");
    auto password = IniHandler::GetInstance().GetValue("dbassistant", "Database", "Password");
    auto database = IniHandler::GetInstance().GetValue("dbassistant", "Database", "DbName", "ORCL");
    // �������ݿ�
    auto db = QSqlDatabase::addDatabase("QOCI");
    db.setHostName(host);
    db.setPort(port);
    db.setUserName(user);
    db.setPassword(password);
    db.setDatabaseName(database);
    LOG(INFO) << X("�����ݿ�:%1,%2,%3,%4,%5,%6")
        .arg(host)
        .arg(port)
        .arg(user)
        .arg(password)
        .arg(database)
        .arg(db.driverName());
    if(!db.open())
    {
        LOG(ERROR) << X("������ʧ��:") << db.lastError().text();
        return false;
    }
    else
    {
        LOG(INFO) << X("�����ݳɹ�!");
        SetSqlDatabase(db);
        return true;
    }
}

bool OracleHandler::Close()
{
    auto db = GetSqlDatabase();
    if(db.isOpen())
    {
        LOG(INFO) << X("�ر����ݿ�:") << db.databaseName();
        db.close();
    }
    return true;
}

bool OracleHandler::IsEnable()
{
    auto db = GetSqlDatabase();
    return db.isOpen();
}

bool OracleHandler::ExeSql(const QString& sql, QString& error)
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

bool OracleHandler::QuerySql(const QString& sql, QSqlQuery& query, QString& error)
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

