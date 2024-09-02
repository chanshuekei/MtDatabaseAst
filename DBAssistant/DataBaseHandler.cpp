#include "DataBaseHandler.h"
#include "easylogging++.h"
#pragma execution_character_set("utf-8")

#ifndef X
#define X(log) QString::fromLocal8Bit(log)
#endif

bool DataBaseHandler::Open(const QString& driver, const QString& dbName, const QString& host = QString(), int port = 0, const QString& user = QString(), const QString& password = QString())
{
    if(m_db.isOpen() || m_db.isValid())
    {
        LOG(DEBUG) << X("关闭数据库:") << m_db.databaseName();
        m_db.close();
    }

    // 连接数据库
    m_db = QSqlDatabase::addDatabase(driver);
    m_db.setUserName(user);

    if(!host.isEmpty() && port > 0)
    {
        m_db.setHostName(host);
        m_db.setPort(port);
    }
    if(!user.isEmpty() && !password.isEmpty())
    {
        // SQLITE数据库不需要用户密码登录
        m_db.setPassword(password);
        m_db.setDatabaseName(dbName);
    }

#if 0
    LOG(INFO) << X("打开数据库:%1,%2,%3,%4,%5,%6")
        .arg(host)
        .arg(port)
        .arg(user)
        .arg(password)
        .arg(dbName)
        .arg(driver);
#endif

    if(!m_db.open())
    {
        LOG(ERROR) << X("打开数据失败:") << m_db.lastError().text();
        return false;
    }
    else
    {
        LOG(INFO) << X("打开数据成功!");
        return true;
    }
}

QSqlDatabase& DataBaseHandler::GetSqlDatabase()
{
    return m_db;
}

void DataBaseHandler::SetSqlDatabase(const QSqlDatabase& db)
{
    if(m_db.isOpen() || m_db.isValid())
    {
        LOG(DEBUG) << X("关闭数据库:") << m_db.databaseName();
        m_db.close();
    }

    LOG(DEBUG) << X("设置数据库:") << db.databaseName();
    m_db = db;
}
