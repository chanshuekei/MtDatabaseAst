#include "SqliteHandler.h"
#include "IniHandler.h"
#include "easylogging++.h"
#pragma execution_character_set("utf-8")

#ifndef X
#define X(log) QString::fromLocal8Bit(log)
#endif // X

SqliteHandler::SqliteHandler()
{}

SqliteHandler::~SqliteHandler()
{}

bool SqliteHandler::Open()
{
    // ��ȡ�����ļ���Ϣ
    auto database = IniHandler::GetInstance().GetValue("dbassistant", "Database", "DbName", "test.db");
    // �������ݿ�
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(database);

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

bool SqliteHandler::Close()
{
    auto db = GetSqlDatabase();
    if(db.isOpen())
    {
        LOG(INFO) << X("�ر����ݿ�:") << db.databaseName();
        db.close();
    }
    return true;
}

bool SqliteHandler::IsEnable()
{
    auto db = GetSqlDatabase();
    return db.isOpen();
}

bool SqliteHandler::ExeSql(const QString& sql, QString& error)
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

bool SqliteHandler::QuerySql(const QString& sql, QSqlQuery& query, QString& error)
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
